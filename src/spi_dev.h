
#ifndef __spi_dev_h__
#define __spi_dev_h__

class spi_dev_t {
public:
	spi_dev_t() {}

	void select(void);

	void release(void);

	void flush(void);

	bool transceive(void * out_data, void * in_data, uint32_t count);

	uint32_t send(uint32_t data);

	bool send_async(uint32_t data);

	bool read_async(uint32_t * data);

	bool is_busy(bool);
};

#endif /* __spi_dev_h__ */
