#ifndef _HDCP_KEY_H_
#define _HDCP_KEY_H_

#ifdef __cplusplus
extern "C"{
#endif

struct gxav_hdcpkey_register {
	unsigned char*       start;
	unsigned int         size;
	unsigned int         encrypt;
};

struct gxav_hdcpkey_info {
	unsigned char* key_addr;
	unsigned int   key_size;
	unsigned int   active;
	unsigned int   encrypt;
};

int  gxav_hdcp_key_init(void);
void gxav_hdcp_key_uninit(void);
int  gxav_hdcp_key_register(struct gxav_hdcpkey_register *param);
int  gxav_hdcp_key_fecth(unsigned int* addr, unsigned int* size);

#ifdef __cplusplus
}
#endif

#endif
