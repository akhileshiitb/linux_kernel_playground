#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>

int main() {
	int ret;

	/* Mount VFS */
	ret = mount("proc", "/proc", "proc", 0, NULL);
	if (ret)
		printf("Error mounting procfs\n");

	ret = mount("sysfs", "/sys", "sysfs", 0, NULL);
	if (ret)
		printf("Error mounting sysfs\n");

	return 0;
}
