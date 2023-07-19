#ifndef _NFC_H_
#define _NFC_H_

#include "NT3H.h"

#define TEXT "Welcome to BearPi-HM_Nano!"//之后暂时不用
#define WEB "huaweicloud.com/?ticket=ST-92234539-Wl4wrigDhhnPsWmyqOE5tZn7-sso"//"openharmony.cn"
#define WIFI_IOT_I2C_BAUDRATE 400000

/*
 * The function write in the NT3H a new URI Rtd on the required position
 *
 * param:
 *      position: where add the record
 *      http:     the address to write
 *
 */
bool storeUrihttp(RecordPosEnu position, uint8_t *http);

/*
 * The function write in the NT3H a new Text Rtd on the required position
 *
 * param:
 *      position: where add the record
 *      text:     the text to write
 *
 */
bool storeText(RecordPosEnu position, uint8_t *text);

#endif /* NFC_H_ */
