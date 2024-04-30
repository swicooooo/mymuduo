#include <mymuduo/Logger.h>

void testLog()
{
	Logger::instance().asyncWrite(0, "hello %s", "world!");
	Logger::instance().asyncWrite(0, "hello %s", "world!");
	Logger::instance().stop();
}

int main()
{
	testLog();
	return 0;
}
