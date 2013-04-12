obj-$(CONFIG_VIDEO_SAMSUNG) += prebuild.fo

.PHONY mk_fa_rel:
	find \( -name '*.[oasch]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '.tmp_*.o.*' -o -name 'modules.order' -o -name modules.builtin \
		-o -name Makefile \) -type f -print | xargs rm -f
	cp Makefile.priv Makefile -af

