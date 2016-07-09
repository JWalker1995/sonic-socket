#include <vector>
#include <deque>
#include <assert.h>

template <typename MessageIdType, unsigned int time_steps_per_second>
class PacketScheduler
{
public:
	class PacketContentsIterator
	{
	};

	PacketScheduler(unsigned int packet_metadata_size)
		: send_size(packet_metadata_size)
    {}

    void send_message(unsigned int size)
	{
		// Notifies that the message is generated and can be sent now.
		// After this method is called, caller must not read or write entry again.

        send_size += size;
	}

	void decode_bytes(unsigned int bytes)
	{
		send_size -= bytes;
		assert(static_cast<signed int>(send_size) >= 0);
	}

	unsigned int get_sleep_duration()
	{
		unsigned int cur_time = get_cur_time();
		unsigned int min_interval_bucket = get_interval_bucket(cur_time - last_send_time);
		last_send_time = cur_time;

		unsigned int max_size_bucket = get_size_bucket(send_size);

		for (unsigned int i = min_interval_bucket; i < num_interval_buckets; i++)
		{
			for (unsigned int j = 0; j <= max_size_bucket; i++)
			{
				Bucket &bucket = get_bucket(i, j);
			}
		}
	}

	PacketContentsIterator send_packet()
	{
	}

private:
	static constexpr unsigned int num_interval_buckets = 32;
	static constexpr unsigned int num_size_buckets = 32;

	// Interval is gradual
	// Size might have jumps, but should occur in blocks

	// Each MessageDesc:
	// f(next_time - prev_time, next_priority, priority, ) -> prev_time offset to send packet

	// Interval
	// Size 

	struct Bucket
	{
		unsigned int sent = 0;
		unsigned int received = 0;
		unsigned int latency_sum = 0;

		float recv_prob = 1.0f;
		float avg_latency = 0.0f;
	};
	Bucket *buckets;

	jw_util::Pool<MessageDesc, true> entry_pool;

	unsigned int last_send_time = 0;
	unsigned int send_size;


	static unsigned int get_cur_time()
	{
		assert(false);
		return 0;
	}

	static unsigned int get_interval_bucket(unsigned int interval)
	{
		unsigned int ms = interval * 1000 / time_steps_per_second;
		unsigned int bucket = jw_util::FastMath::log2(ms * ms);
		if (bucket >= num_interval_buckets) {bucket = num_interval_buckets - 1;}
		return bucket;
	}

	static unsigned int get_size_bucket(unsigned int size)
	{
		unsigned int bucket = jw_util::FastMath::log2(size * size);
		if (bucket >= num_size_buckets) {bucket = num_size_buckets - 1;}
		return bucket;
	}

	Bucket &get_bucket(unsigned int interval_bucket, unsigned int size_bucket) const
	{
		assert(interval_bucket < num_interval_buckets);
		assert(size_bucket < num_size_buckets);

		unsigned int index = interval_bucket * num_size_buckets + size_bucket;
		return buckets[index];
	}
};


/*
Send init, and keep sending inits every second until remote acknowledges
On recv init, send ack
When remote has acked and we have acked, begin sending encoded data
Keep track of probability that remote has decoded all packets. If probability drops below a threshold,

Keep table of the reception ratio and latency of packets sent on a x-ms interval, of size y bytes.

Minimize: sum of weighted message reception delay squared

Prioritization use case:
	Terrain generation is lower priority than entity control
*/
