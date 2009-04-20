/* debug macros */
#define MFC_DEBUG(fmt, ...)					\
	do {							\
		printk(KERN_DEBUG				\
			"%s: " fmt, __func__, ##__VA_ARGS__);	\
	} while(0)

#define MFC_ERROR(fmt, ...)					\
	do {							\
		printk(KERN_ERR					\
			"%s: " fmt, __func__, ##__VA_ARGS__);	\
	} while (0)

#define MFC_NOTICE(fmt, ...)					\
	do {							\
		printk(KERN_NOTICE				\
			fmt, ##__VA_ARGS__);			\
	} while (0)

#define MFC_INFO(fmt, ...)					\
	do {							\
		printk(KERN_INFO				\
			fmt, ##__VA_ARGS__);			\
	} while (0)

#define MFC_WARN(fmt, ...)					\
	do {							\
		printk(KERN_WARNING				\
			fmt, ##__VA_ARGS__);			\
	} while (0)


#ifdef VIDEO_MFC_DEBUG
#define mfc_debug(fmt, ...)		MFC_DEBUG(fmt, ##__VA_ARGS__)
#else
#define mfc_debug(fmt, ...)
#endif

#define mfc_err(fmt, ...)		MFC_ERROR(fmt, ##__VA_ARGS__)
#define mfc_notice(fmt, ...)		MFC_NOTICE(fmt, ##__VA_ARGS__)
#define mfc_info(fmt, ...)		MFC_INFO(fmt, ##__VA_ARGS__)
#define mfc_warn(fmt, ...)		MFC_WARN(fmt, ##__VA_ARGS__)
