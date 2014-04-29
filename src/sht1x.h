
#ifndef __sht1x_h__
#define __sht1x_h__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct sh1x {
	void * priv;
	uint8_t	(*read_temperature)	(const struct sh1x * self);
	uint8_t	(*read_moisture)	(const struct sh1x * self);
	void	(*set_status)		(const struct sh1x * self);
	uint8_t	(*get_status)		(const struct sh1x * self);
} sht1x_t;


bool sht1x_init(sht1x_t * self);


#ifdef __cplusplus
}
#endif

#endif /* __sht1x_h__ */
