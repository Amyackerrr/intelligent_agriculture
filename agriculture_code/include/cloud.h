typedef struct { // object data type
    char *Buf;
    uint8_t Idx;
} MSGQUEUE_OBJ_t;


typedef enum {
    en_msg_cmd = 0,
    en_msg_report,
} en_msg_type_t;

typedef struct {
    char *request_id;
    char *payload;
} cmd_t;

typedef struct {
    int lum;
    int temp;
    int hum;
} report_t;


typedef struct {
    en_msg_type_t msg_type;
    union {
        cmd_t cmd;
        report_t report;
    } msg;
} app_msg_t;

typedef struct {
    int connected;
    int led;
    int motor;
} app_cb_t;

void ReportMsg(report_t *report);
void oc_cmdresp(cmd_t *cmd, int cmdret);
void DealCmdMsg(cmd_t *cmd);