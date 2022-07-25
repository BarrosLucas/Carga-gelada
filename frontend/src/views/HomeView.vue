<template>
  <div class="home">
    <v-container fluid>
      <v-row justify="center" class="header">
        <v-col cols="12" md="8">
          <v-banner
            color="#008b8b"
            elevation="7"
            shaped
            height="40"
            dark
            width="400"
          >
            <h3 style="padding: 10px 5px">
              Carga Gelada <v-icon>mdi-cube</v-icon>
            </h3>
          </v-banner>
        </v-col>
        <v-col cols="12" md="4">
          <v-btn color="#008b8b" elevation="3" icon outlined @click="getData">
            <v-icon dark> mdi-autorenew </v-icon>
          </v-btn>
        </v-col>
      </v-row>
      <v-divider></v-divider>
      <v-row class="data-titles">
        <v-col>
          <label for=""> Logs Registrados</label>
        </v-col>
        <v-col>
          <label for=""> Variação de temperatura</label>
        </v-col>
      </v-row>
      <v-row justify="center" class="content">
        <v-col cols="12" md="6">
          <v-data-table
            :headers="headers"
            :items="temperaturas"
            :items-per-page="10"
            class="elevation-1"
          ></v-data-table>
        </v-col>
        <v-col v-if="showGraph" cols="12" md="6">
          <v-sparkline
            :fill="fill"
            :gradient="gradients"
            :line-width="width"
            :padding="padding"
            :smooth="radius || false"
            :value="temperaturasValues"
            auto-draw
            show-labels
            auto-line-width
            label-size="5"
          ></v-sparkline>
        </v-col>
      </v-row>
    </v-container>
    <FooterComponent />
  </div>
</template>

<script>
import axios from "axios";
import FooterComponent from "../components/Footer.vue";
export default {
  name: "Home",
  components: { FooterComponent },
  data() {
    return {
      temperaturas: [],

      headers: [
        { text: "Mensagem  log", value: "message" },
        { text: "Temperatura em ºC", value: "valor" },
        { text: "Motor", value: "motorStatus" },
      ],

      gradients: ["#f72047", "#ffd200", "#1feaea"],
      fill: true,
      padding: 8,
      radius: 10,
      width: 2,
      showGraph: false,
    };
  },
  mounted() {
    this.getData();
  },

  methods: {
    async getData() {
      this.temperaturas = [];
      let res = await axios.get(
        "https://carga-gelada-default-rtdb.firebaseio.com/logs.json"
      );

      const data = res.data;

      Object.values(data).forEach((obj) => {
        obj.motorStatus = obj.motor == 1 ? "Ligado" : "Desligado";
        this.temperaturas.push(obj);
      });

      this.showGraph = true;
    },
  },
  computed: {
    temperaturasValues() {
      const temps = [];

      this.temperaturas.forEach((element) => {
        temps.push(element.valor);
      });

      return temps;
    },
  },
};
</script>

<style scoped>
.header {
  margin: 15px auto;
  display: flex;
  align-items: center;
  justify-content: center;
}
.content {
  margin: 30px auto;
  padding: 5px 30px;
}
.data-titles {
  margin: 20px auto;
  padding: 10px 30px;
  color: #008b8b;
  font-size: large;
}

.tolip {
  border: 1px solid red;
}
</style>
