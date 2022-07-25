<template>
  <v-footer light padless>
    <v-card flat tile class="indigo lighten-1 white--text text-center">
      <v-card-text>
        <v-btn class="mx-4 white--text" icon>
          <a
            href="https://github.com/gabrielrfonseca/Carga-gelada"
            target="_blank"
            rel="noopener noreferrer"
          >
            <v-icon size="24px">
              {{ icon }}
            </v-icon></a
          >
        </v-btn>
        <v-btn class="mx-4 white--text" icon>
          <a
            href="https://github.com/invaderZi/carga-gelada-vue"
            target="_blank"
            rel="noopener noreferrer"
          >
            <v-icon size="24px">
              {{ iconvue }}
            </v-icon></a
          >
        </v-btn>
      </v-card-text>

      <v-card-text class="white--text pt-0">
        O projeto "Carga gelada" foi desenvolvido para a turma de
        Microcontroladores - UFPB. Ele consiste em um monitorador de
        temperatura, com o próposito de ser utilizados em caminhões baú que
        fazem o transporte de frutas.
      </v-card-text>

      <v-divider></v-divider>

      <v-card-text class="white--text">
        {{ new Date().getFullYear() }} — <strong>Carga Gelada</strong>
      </v-card-text>
    </v-card>
  </v-footer>
</template>
<script>
export default {
  data: () => ({
    icon: "mdi-github",
    iconvue: "mdi-vuejs",
  }),
};
</script>
