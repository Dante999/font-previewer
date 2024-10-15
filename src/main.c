#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>

#include "dynamic_array.h"
#include "util_makros.h"

#define FONT_DIR_FILTER  "0_for-comercial-use"
#define OUTPUT_FILE      "preview.svg"
#define FONT_SIZE        "18" 
#define CHAR_WIDTH_FACTOR  12
#define CHAR_HEIGHT_FACTOR 26


struct font {
	char name[255];
	char style[255];
};


struct font_list {
	struct font *items;
	size_t count;
	size_t capacity;
};

void font_print_list(struct font_list *f)
{
	for (size_t i=0; i < f->count; ++i) {
		printf("%zu: %s, %s\n", i, f->items[i].name, f->items[i].style);
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

size_t get_longest_font_name(struct font_list *fonts)
{
	size_t max_len = 0;

	for (size_t i=0; i < fonts->count; ++i) {
		max_len = MAX(max_len, strlen(fonts->items[i].name));
	}

	printf("max font length: %zu chars\n", max_len);
	return max_len;
}

void write_svg(FILE *svg, struct font_list *fonts, const char *text)
{
	size_t max_fontname_len = get_longest_font_name(fonts);

	printf("text to print: %s\n", text);

	size_t doc_width  = (strlen(text) + max_fontname_len) * CHAR_WIDTH_FACTOR;
	size_t doc_height = fonts->count* CHAR_HEIGHT_FACTOR; 

	svg_append(svg, "<?xml version=`1.0` encoding=`UTF-8` standalone=`no`?>\n");
	svg_append(svg, "<svg\n");
	svg_append(svg, "\twidth=`%zu`\n", doc_width);
	svg_append(svg, "\tweight=`%zu`\n", doc_height);
	svg_append(svg, "\tviewBox=`0 0 %zu %zu`>\n", doc_width, doc_height);
	
	for (size_t i=0; i < fonts->count; ++i) {

		struct font *font = &fonts->items[i];

		char font_family[max_fontname_len+3];
		strncpy(font_family, font->name, sizeof(font_family));

		for (size_t j=strlen(font_family); j < sizeof(font_family); ++j) {
			font_family[j] = '.';
		}
		font_family[sizeof(font_family)-1] = '\0';

		const size_t x_offset = max_fontname_len * CHAR_WIDTH_FACTOR;
		const size_t y_offset = i * CHAR_HEIGHT_FACTOR;

		svg_append(svg, "\t\t<g>\n");
		svg_append(svg, "\t\t\t<text x=`0` y=`%d` font-size=`%s` font-family=`mono`>%s</text>\n",
				10+y_offset, FONT_SIZE, font_family);

		svg_append(svg, "\t\t\t<text x=`%d` y=`%d` font-size=`%s` font-family=`%s` font-weight=`%s`>%s</text>\n",
				x_offset, 10+y_offset, FONT_SIZE, font->name, font->style, text);

		svg_append(svg, "\t\t</g>\n\n");
	}

	svg_append(svg, "</svg>\n");
}

void load_all_fonts_from_dir(struct font_list *fonts, const char *directory)
{
	char cmd[2048];

	snprintf(cmd, sizeof(cmd), "fc-list --format=\"%%{family[0]}: %%{file}\\n\" | grep %s | cut -d: -f1 | sort | uniq > /tmp/fonts.txt", directory);
	system(cmd);

	FILE *fonts_file = fopen("/tmp/fonts.txt", "r");

	if( fonts_file == NULL) {
		int err = errno;
		fprintf(stderr, "unable to open fonts.txt: %s", strerror(err));
		return;
	}

	char linebuffer[1024];

	while (fgets(linebuffer, sizeof(linebuffer), fonts_file) ) {

		if (strlen(linebuffer) == 0) {
			continue;
		}

		char *newline = strchr(linebuffer, '\n');
		if (newline != NULL) {
			*newline = '\0';
		}

		struct font tmp;
		strncpy(tmp.name, linebuffer, sizeof(tmp.name));
		strncpy(tmp.style, "normal", sizeof(tmp.style));
		da_append(fonts, tmp);
	}

}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "ERROR: no text given!\n");
		fprintf(stderr, "usage: %s <text>\n", argv[0]);
		return -1;
	}

	printf("%s\n", argv[0]);

	struct font_list fonts = {0};

	load_all_fonts_from_dir(&fonts, FONT_DIR_FILTER);

	font_print_list(&fonts);

	char svg_path[PATH_MAX];
	getcwd(svg_path, sizeof(svg_path));

	strcat(svg_path, "/" OUTPUT_FILE);
	FILE *svg = fopen(svg_path, "w");

	if (svg == NULL) {
		fprintf(stderr, "ERROR: unable to create file %s\n", svg_path);
		return -1;
	}

	write_svg(svg, &fonts, argv[1]);


	printf("=> svg with all fonts created at: %s\n", svg_path); 
	fclose(svg);

	da_free(&fonts);
}
