#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "ds18b20.h"
#include <sys/param.h>
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "my_data.h"


// ------------------------------------------------------------------
//........................ defines wifi
int enviar_nuvem = 0;
char *mensagem_nuvem;
char temperatura_em_char[60];

// Define client certificate
extern const uint8_t certificate_pem_start[] asm("_binary_certificate_pem_start");
extern const uint8_t certificate_pem_end[]   asm("_binary_certificate_pem_end");

// .....................   defines sdcard
static const char *TAG = "status";

#define MOUNT_POINT "/sdcard"
#define PIN_NUM_MISO  CONFIG_EXAMPLE_PIN_MISO
#define PIN_NUM_MOSI  CONFIG_EXAMPLE_PIN_MOSI
#define PIN_NUM_CLK   CONFIG_EXAMPLE_PIN_CLK
#define PIN_NUM_CS    CONFIG_EXAMPLE_PIN_CS

// ...................... defines temeperatura 
#define TEMP_BUS 13
#define LED 17
#define HIGH 1
#define LOW 0
#define digitalWrite gpio_set_level

DeviceAddress tempSensors[1];

//INTERRUPCAO BOTAO
static QueueHandle_t gpio_isr_queue = NULL;

int sign = 0; //estado do motor

// WiFi
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    switch (event_id){
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection(){
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

// ..................  funcoes client firebase
// Client
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id){
    case HTTP_EVENT_ON_DATA:
        printf("Client HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

//metodo post firebase
static void client_post_rest_function(char *mensagem)
{
    esp_http_client_config_t config_post = {
        .url = "https://carga-gelada-default-rtdb.firebaseio.com/logs.json",
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char *)certificate_pem_start,
        .event_handler = client_event_get_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);


	char *post_data = mensagem;
	
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);

	enviar_nuvem = 0; // se enviou ele reseta a variavel de enviar
	
}

void montastringfrebase (float temperatura){

 	sprintf(temperatura_em_char,"{\"message\":\"Temperatura Elevada!\",\"valor\":%.2f,\"motor\":%d}",temperatura,sign);
	mensagem_nuvem = temperatura_em_char;
	printf("ENVIANDO PARA NUVEM:   %s", mensagem_nuvem);

}


//................................ funcoes motor
/*ISR PARA O MOTOR*/
static void IRAM_ATTR gpio_isr_handler(void *arg){
	xQueueSendFromISR(gpio_isr_queue, arg, NULL);
}


static void interrupt_task(void *arg){
	int pin = 0;
	while(1){

		if(xQueueReceive(gpio_isr_queue, &pin, portMAX_DELAY)){
			printf("\n Mudança no motor \n");
			sign = !sign;
			gpio_set_level(GPIO_NUM_2, sign);
		}
	}
}

// ........................   funcoes temperatura sensor
void getTempAddresses(DeviceAddress *tempSensorAddresses) {
	unsigned int numberFound = 0;
	reset_search();
	while (search(tempSensorAddresses[numberFound],true)) {
		numberFound++;
		if (numberFound == 1) break;
	}
	
	return;
}


void app_main(void){

    // queue motor
    gpio_isr_queue = xQueueCreate(10, sizeof(int));

    // gpio motor
	gpio_config_t io_conf = {};
		
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = 1ULL << 5;
	io_conf.pull_down_en = 1;
	gpio_config(&io_conf);

	
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1ULL << 2;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	
	gpio_install_isr_service(0);

	// gpio  temperatura sensor

    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
	ds18b20_init(TEMP_BUS);
    getTempAddresses(tempSensors);
    ds18b20_setResolution(tempSensors,2,10);

	//adress temperatura
	printf("Address 0: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", tempSensors[0][0],tempSensors[0][1],tempSensors[0][2],tempSensors[0][3],tempSensors[0][4],tempSensors[0][5],tempSensors[0][6],tempSensors[0][7]);
	



    // Motor task
	gpio_isr_handler_add(GPIO_NUM_5, gpio_isr_handler, &sign);	 
	
	xTaskCreate(interrupt_task, "interrupt", 2048, NULL, 1, NULL);


	// SD Card Configuracao
	esp_err_t ret;

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
	#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
			.format_if_mount_failed = true,
	#else
			.format_if_mount_failed = false,
	#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
			.max_files = 5,
			.allocation_unit_size = 16 * 1024
	};
	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");

	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.max_freq_khz = SDMMC_FREQ_PROBING;
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = PIN_NUM_MISO,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 19000,
	};
	ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return;
	}

	
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = PIN_NUM_CS;
	slot_config.host_id = host.slot;

	ESP_LOGI(TAG, "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
					"If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
					"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return;
	}
	ESP_LOGI(TAG, "Filesystem mounted");

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);
	// Use POSIX and C standard library functions to work with files.

	// First create a file.
	const char *file_hello = MOUNT_POINT"/log.txt";

	ESP_LOGI(TAG, "Opening file %s", file_hello);
	FILE *f = fopen(file_hello, "a");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for writing");
		return;
	}


    // task for temperature sensor e print status do motor

	float temperatura_nuvem = 0.00;

	while (1) {
		ds18b20_requestTemperatures();

		float temp1 = ds18b20_getTempF((DeviceAddress *)tempSensors[0]);
		float temp3 = ds18b20_getTempC((DeviceAddress *)tempSensors[0]);
        
		float cTemp = ds18b20_get_temp();

		if(cTemp > 27.2){

			
				fprintf(f, "Temperatura elevada: %.2fC\tEstado do Ar Condicionado: %d\n", cTemp,sign);
				enviar_nuvem = 1;
				temperatura_nuvem = cTemp;				
				fclose(f);
				ESP_LOGI(TAG, "Temperatura elevada: %.2fC\n", cTemp);

			
			printf("Temperatura elevada.\n");

			
		}

		printf("Temperatura: %0.1fC\n", cTemp); // print temperatura
		printf("Motor status: %d\n",sign);

		vTaskDelay(30000 / portTICK_PERIOD_MS);

		if(enviar_nuvem){

		vTaskDelay(2000 / portTICK_PERIOD_MS);

		printf("Montano dados para nuvem ...........\n\n");

		montastringfrebase(temperatura_nuvem);

		vTaskDelay(2000 / portTICK_PERIOD_MS);
		// se gravar no cartao sd
		nvs_flash_init();
		wifi_connection();

		vTaskDelay(2000 / portTICK_PERIOD_MS);
		printf("WIFI was initiated ...........\n\n");

		vTaskDelay(2000 / portTICK_PERIOD_MS);
		printf("Start client:\n\n");
		client_post_rest_function(mensagem_nuvem);

		vTaskDelay(5000 / portTICK_PERIOD_MS);
		esp_wifi_disconnect(); //desconecta da wifi 5 segundos depois de enviar
		printf("Wifi disconected...........#####:\n\n");

	 }
	}

	// All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);

	

}