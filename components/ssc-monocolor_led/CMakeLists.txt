idf_component_register( SRCS "monocolor_led.c"
                        INCLUDE_DIRS "."
                        REQUIRES ssc-mqtt_connection
                        PRIV_REQUIRES ssc-dimm_storage)