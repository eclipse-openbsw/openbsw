// necessary on --spec=nosys.spec and C++ where no _fini is available from libc

extern "C"
void _fini()
{
}


