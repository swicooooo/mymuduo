#include "Timestamp.h"

#include <ctime>
#include <iomanip>
#include <sstream>

Timestamp::Timestamp(): microsecondsSinceEpoch_(0){}

Timestamp::Timestamp(int64_t microsecondsSinceEpoch):microsecondsSinceEpoch_(microsecondsSinceEpoch){}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}

std::string Timestamp::toString()
{
    std::ostringstream oss; 
    oss << std::put_time(localtime(&microsecondsSinceEpoch_),"%F %T");
    return oss.str();
}
