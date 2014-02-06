#include "dlfcn.h"
#include "stdio.h"



int main()
{
	int err = 0;
	char* dl_err;
	void* hdl = NULL;

	int (*reset_mtcat) (unsigned char);
	int (*reset_all) ();
	int (*load_crate_mask) (unsigned int, unsigned char);
	int (*hv_stop_ok) ();

	printf("dlopen?\n");

	hdl = dlopen("./libmtcat_lj.so", RTLD_LAZY);
	if (hdl == NULL) {
		printf(".so not found\n");
		fputs (dlerror(), stderr);
		hdl = dlopen("libmtcat_lj.dylib", RTLD_LAZY);
		if (hdl == NULL) return -1;
	}
	dlerror();

	printf("lib found.\n");

	reset_mtcat = dlsym(hdl, "reset_mtcat");
	if ((dl_err = dlerror()) != NULL) {
		fputs(dl_err, stderr);
		return -1;
	}

	printf("reset_mtcat\n");

	reset_all = dlsym(hdl, "reset_all");
	if ((dl_err = dlerror()) != NULL) {
		fputs(dl_err, stderr);
		return -1;
	}

	printf("reset_all\n");

	load_crate_mask = dlsym(hdl, "load_crate_mask");
	if ((dl_err = dlerror()) != NULL) {
		fputs(dl_err, stderr);
		return -1;
	}

	printf("crate_mask\n");

    hv_stop_ok = dlsym(hdl, "hv_stop_ok");
	if ((dl_err = dlerror()) != NULL) {
		fputs(dl_err, stderr);
		return -1;
	}

	printf("hv_stop_ok\n");


	//err = reset_mtcat(0);
	//err = reset_all();
	//err = load_crate_mask(0, 0);
	//err = load_crate_mask(0x00040000, 1);
	//err = load_crate_mask(0xffffbfff, 0);

	//if (err) printf("failed.\n");
	//printf("ok.\n");

    err = hv_stop_ok();
    if (err < 0) printf("failed.\n");
    else if (err == 0) printf("mushroom smashed.\n");
    else if (err == 1) printf("mushroom happy.\n");
    else printf("mushroom got sick.\n");

    dlclose(hdl);
	return 0;
}

