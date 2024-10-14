#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#define OUTPUT_FILE      "preview.svg"
#define MAX_NUM_OF_FONTS 255
#define FONT_SIZE        "18" 
#define CHAR_WIDTH_FACTOR  12
#define CHAR_HEIGHT_FACTOR 20







struct fonts {
	char   *names[MAX_NUM_OF_FONTS];
	size_t amount;
};

struct fonts g_font_list;

void font_init(struct fonts *f)
{
	f->amount = 0;
}

void font_free(struct fonts *f)
{
	for (size_t i=0; i < f->amount; ++i) {
		free(f->names[i]);
	}

	f->amount = 0;
}

void font_append(struct fonts *f, const char *path)
{
	if (f->amount >= MAX_NUM_OF_FONTS) {
		fprintf(stderr, "ERROR: maximum amount of %d fonts reached!\n",
				(int) MAX_NUM_OF_FONTS);
		return;
	}

	const size_t needed_size = strlen(path)+1;

	f->names[f->amount] = (char*) malloc(needed_size);

	strncpy(f->names[f->amount], path, needed_size);
	++f->amount;
}

void font_print_list(struct fonts *f)
{
	for (size_t i=0; i < f->amount; ++i) {
		printf("%zu: %s\n", i, f->names[i]);
	}
}




static void svg_append(FILE *svg, const char *fmt, ...)
{
	char buffer[1024];

	va_list arg_list;
	va_start(arg_list, fmt);

	vsnprintf(buffer, sizeof(buffer), fmt, arg_list);

	for (size_t i=0; i < strlen(buffer); ++i) {

		if (buffer[i] == '`') {
			buffer[i] = '\"';
		}
	}

	fprintf(svg, "%s", buffer);

	va_end(arg_list);
}

size_t get_longest_font_name(struct fonts *fonts)
{
	size_t biggest_len = 0;

	for (size_t i=0; i < fonts->amount; ++i) {

		const size_t tmp_len = strlen(fonts->names[i]);

		biggest_len = biggest_len > tmp_len ? biggest_len : tmp_len;
	}

	printf("biggest font_len: %zu\n", biggest_len);
	return biggest_len;
}

void write_svg(FILE *svg, struct fonts *fonts, const char *text)
{
	size_t max_fontname_len = get_longest_font_name(fonts);

	printf("text to print: %s\n", text);

	size_t doc_width  = (strlen(text) + max_fontname_len) * CHAR_WIDTH_FACTOR;
	size_t doc_height = fonts->amount * CHAR_HEIGHT_FACTOR; 

	svg_append(svg, "<?xml version=`1.0` encoding=`UTF-8` standalone=`no`?>\n");
	svg_append(svg, "<svg\n");
	svg_append(svg, "\twidth=`%zu`\n", doc_width);
	svg_append(svg, "\tweight=`%zu`\n", doc_height);
	svg_append(svg, "\tviewBox=`0 0 %zu %zu`>\n", doc_width, doc_height);
	
	for (size_t i=0; i < fonts->amount; ++i) {

		char font_family[max_fontname_len+3];
		strncpy(font_family, fonts->names[i], sizeof(font_family));

		for (size_t j=strlen(font_family); j < sizeof(font_family); ++j) {
			font_family[j] = '.';
		}
		font_family[sizeof(font_family)-1] = '\0';

		const size_t x_offset = max_fontname_len * CHAR_WIDTH_FACTOR;
		const size_t y_offset = i * CHAR_HEIGHT_FACTOR;

		svg_append(svg, "\t\t<text x=`0` y=`%d` font-size=`%s` font-family=`mono`>%s</text>\n",
				10+y_offset, FONT_SIZE, font_family);

		svg_append(svg, "\t\t<text x=`%d` y=`%d` font-size=`%s` font-family=`%s`>%s</text>\n\n",
				x_offset, 10+y_offset, FONT_SIZE, fonts->names[i], text);


	}

	svg_append(svg, "</svg>\n");
}

/*
void on_file_callback(const char *filename)
{
	if (strstr(filename, ".ttf") != NULL) {
//		printf("->%s\n", filename);
		font_append(&g_font_list, filename);
	}
}

void search_for_files(const char *dirname, void (*on_file)(const char *filename))
{
	DIR *dr = opendir(dirname);

	if (dr == NULL) {
		int error = errno;
		fprintf(stderr, "unable to open %s: %s\n", dirname, strerror(error));
		return;
	}

	struct dirent *dir_entry;


	chdir(dirname);
	while ((dir_entry = readdir(dr)) != NULL) {

		const char *filename = dir_entry->d_name;
		struct stat filestat;

		stat(filename, &filestat);

		if (strcmp(filename, ".") == 0 || 
		    strcmp(filename, "..") == 0) {
			continue;
		}
		else if (S_ISDIR(filestat.st_mode)) {
			search_for_files(filename, on_file);
		}
		else if (S_ISREG(filestat.st_mode)) {
			on_file(filename);
		}
	}

	chdir("..");

	closedir(dr);
}
*/

void load_all_fonts_from_dir(struct fonts *fonts, const char *directory)
{
	char cmd[1024];

	// TODO: determiny style with %%{style} and add this info to font list
	// or at least remove font-family duplicates
	snprintf(cmd, sizeof(cmd), "fc-list --format=\"%%{family[0]}: %%{file}\\n\" | grep %s | sort | uniq > fonts.txt", directory);
	system(cmd);

	FILE *fonts_file = fopen("fonts.txt", "r");

	if( fonts_file == NULL) {
		int err = errno;
		fprintf(stderr, "unable to open fonts.txt: %s", strerror(err));
		return;
	}

	char linebuffer[1024];

	while (fgets(linebuffer, sizeof(linebuffer), fonts_file) ) {
		
		char *delim = strchr(linebuffer, ':');

		if (delim != NULL) {
			*delim = '\0';
			font_append(fonts, linebuffer);
		}
	}
	//	printf("command: %s\n", cmd);

}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "ERROR: no text given!\n");
		fprintf(stderr, "usage: ./%s <text>\n", argv[0]);
		return -1;
	}

	printf("%s\n", argv[0]);

	char workdir[PATH_MAX];

	if (getcwd(workdir, sizeof(workdir)) == NULL) {
		fprintf(stderr, "unable to store current workingdir!\n");
		return -1;
	}

	font_init(&g_font_list);

	load_all_fonts_from_dir(&g_font_list, "0_for-comercial-use");
//	chdir(workdir);
#if 0
	font_append(&font_list, "Amatic SC");
	font_append(&font_list, "Alex Brush");
	font_append(&font_list, "Artifika");
	font_append(&font_list, "Baru");
	font_append(&font_list, "BlackChancery");
	font_append(&font_list, "Bougaty");
	font_append(&font_list, "Cadmium Egg");
	font_append(&font_list, "Calamity Jane NF");
	font_append(&font_list, "California");
	font_append(&font_list, "Carolus FG");
	font_append(&font_list, "Clicker Script");
	font_append(&font_list, "Daniel Jaques");
#endif

	font_print_list(&g_font_list);


	FILE *svg = fopen(OUTPUT_FILE, "w");

	if (svg == NULL) {
		fprintf(stderr, "ERROR: unable to create file %s\n", OUTPUT_FILE);
		return -1;
	}

	write_svg(svg, &g_font_list, argv[1]);
	fclose(svg);

	font_free(&g_font_list);
}
