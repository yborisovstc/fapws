#ifndef __FAP_EXT_H
#define __FAP_EXT_H

#include <fapbase.h>

// Plugin iface
//
#if 0
class MAE_Plugin
{
    public: 
	virtual TTransInfo** GetTinfos() = 0;
};
#endif

// Each plugin has to export the function named "init" with this type
//
typedef CAE_ProviderBase* plugin_init_func_t();


#endif
