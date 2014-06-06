#ifndef __rh_h__
#define __rh_h__

/*
class rf_t {
private:
	ccx_hw_t& hw;

	bool acquire_lock();
	void release_lock();

public:
	rf_t(const ccx_hw_t& ahw ) : hw(ahw) {
	}

	uint8_t version();
	uint8_t part_number();

	int8_t receive(uint8_t * data, uint8_t * data_size, uint8_t * src_addr, uint8_t * dst_addr);
	uint8_t can_receive(portTickType ticks);

	int8_t prepare(const void * payload, uint8_t payload_len);
	int8_t transmit();
	int8_t send(const void * payload, uint8_t payload_len);

	int8_t read(void * buffer, uint16_t buffer_len);
	uint8_t channel_clear();

	uint8_t receiving_packet();
	uint8_t pending_packet();

	uint8_t rx_on();
	uint8_t rx_off();

	uint8_t sleep();
};
*/

#endif /* __rh_h__ */
