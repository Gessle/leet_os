#define SERIALCOMM_WAIT_INTERVAL 1
#define SERIALCOMM_BUFF_LEN 0x100
#define SERIALCOMM_BUFF_CTS_LIMIT 128

struct serport_buffer
{
  unsigned char buffer[SERIALCOMM_BUFF_LEN];  
  volatile unsigned char input_offset;
  unsigned char output_offset;
  volatile unsigned char error;
  unsigned flow_control:2;
  unsigned xoff_received:1;
  unsigned xoff_sent:1;
};
#define SERIAL_PARITY_ERROR 0x01
#define SERIAL_FRAMING_ERROR 0x02
#define SERIAL_BUFFER_FULL 0x04
