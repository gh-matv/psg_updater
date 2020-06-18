#pragma once

#define SG_URL "https://streetgears.eu/index.php"
#define SG_QUERY(controller, action, params, playerkey) \
	std::string(SG_URL "?c=")+controller+"&a="+action+"&t="+playerkey+"&"+params
#define SG_APIURL(action, params, playerkey) SG_QUERY("Api_Patcher", action, params, playerkey)
#define SG_APIPATCHER_QUERY(action, params, playerkey) SG_QUERY("Api_Patcher", "action, params, playerkey")
