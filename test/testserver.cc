#include <mymuduo/Logger.h>

void testLog()
{
	Logger::instance().asyncWrite(0, "hello ");
	Logger::instance().asyncWrite(0, "world!");
	Logger::instance().stop();
}

int main()
{
	testLog();
	return 0;
}
