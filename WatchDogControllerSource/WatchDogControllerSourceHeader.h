// WatchDogControllerSourceHeader.h
#ifndef WATCH_DOG_CONTROLLER_SOURCE_HEADER_H
#define WATCH_DOG_CONTROLLER_SOURCE_HEADER_H

struct WatchDogControllerSourceHeader
{
	enum op_code
	{
		ping = 0,
		pong,
		restart
	};
	op_code op;
	unsigned int size;
};

#endif