# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS "main_final.c" "ds18b20.c"
                    INCLUDE_DIRS  "."
                    EMBED_TXTFILES  "certificate.pem")
#idf_component_register(SRCS "microsd.c"
#                    INCLUDE_DIRS  "")
