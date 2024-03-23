#include "TimeStamp.h"
#include <muduo/base/Timestamp.h>

#include <ctime>
#include <iomanip>
#include <sstream>

TimeStamp::TimeStamp(): microsecondsSinceEpoch_(0){}

TimeStamp::TimeStamp(int64_t microsecondsSinceEpoch):microsecondsSinceEpoch_(microsecondsSinceEpoch){}

TimeStamp TimeStamp::now()
{
    return TimeStamp(time(NULL));
}

std::string TimeStamp::toString()
{
    std::ostringstream oss; 
    oss << std::put_time(localtime(&microsecondsSinceEpoch_),"%F %T");
    return oss.str();
}
