
#ifndef __dma_dev_h__
#define __dma_dev_h__


class dma_error_listener_t {
public:
	virtual void handle_error_isr(uint32_t status) const = 0;
};


class dma_soft_listener_t {
public:
	virtual void handle_soft_isr(uint32_t status) const = 0;
};


class dma_listener_t : public dma_error_listener_t /* is not used at the moment , dma_soft_listener_t*/ {
};


class dma_dev_t {
private:
	std::vector<dma_listener_t*> error_listeners;

	dma_dev_t();
	dma_dev_t(const dma_dev_t& dma_dev_) {}
	void operator=(const dma_dev_t& dma_dev_) {}

public:
	static dma_dev_t& get(void) {
		static dma_dev_t dma_dev;
		return dma_dev;
	}

	void subscribe_error(dma_listener_t * listener);
	bool unsubscribe_error(dma_listener_t * listener);
	virtual void handle_error_isr(void);
};

#endif /* __dma_dev_h__ */
