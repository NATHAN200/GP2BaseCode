#pragma once

class IRenderer
{
public:
	//virtual deconstructor, so this class can be overridden
	virtual ~IRenderer(){};
	//virtual function, can be overridden
	virtual bool init(void *pWindowHandle,bool fullScreen)=0;
	//virtual function, can be overridden
	virtual void clear(float r,float g,float b,float a)=0;
	virtual void render()=0;
	//virtual function, can be overridden
	virtual void present()=0;
};