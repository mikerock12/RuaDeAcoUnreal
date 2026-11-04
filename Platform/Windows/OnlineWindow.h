#pragma once
namespace RDAOnline {
bool Open(void* parent,float master,float music,float effects,float touchOpacity);
bool Active();
void Close();
int Smoke(int port);
}
