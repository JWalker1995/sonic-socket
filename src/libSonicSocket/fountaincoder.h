#ifndef FOUNTAINCODER_H
#define FOUNTAINCODER_H

#include "libSonicSocket/fountainsource.h"
#include "libSonicSocket/fountainsink.h"
#include "libSonicSocket/logproxy.h"

namespace sonic_socket
{

class FountainCoder : public FountainSource<FountainCoder>, public FountainSink<FountainCoder>
{
public:
	FountainCoder(LogProxy &logger)
		: logger(logger)
    {}

    LogProxy &get_logger() {return logger;}

private:
	LogProxy &logger;
};

}

#endif // FOUNTAINCODER_H
