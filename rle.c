#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<limits.h>
#include<stdbool.h>
typedef struct meta_data
{
	char *file_name;
	int size;
	FILE *fpt;
	unsigned char *data;
}m_data;
int get_file_size(char *filename) {
	struct stat file_status;
	if (stat(filename, &file_status) < 0) {
		return -1;
	}
	return file_status.st_size;
}
void read_data(m_data *meta_d)
{
	meta_d->data = (unsigned char*)calloc(meta_d->size, sizeof(unsigned char));
	meta_d->fpt = fopen(meta_d->file_name, "r");
	fread(meta_d->data, sizeof(unsigned char), meta_d->size, meta_d->fpt);
	fclose(meta_d->fpt);
}
void write_data(m_data *meta_d)
{
	meta_d->fpt = fopen(meta_d->file_name, "w");
	fwrite(meta_d->data, sizeof(unsigned char), meta_d->size, meta_d->fpt);
	fclose(meta_d->fpt);
}
void rle_compress(m_data *in_data, m_data *comp_data)
{
	comp_data->data = (unsigned char*)calloc(in_data->size*2, sizeof(unsigned char));
	int i = 0, n=0;
	unsigned char c, count = 0, rle_len = 4;
	bool rle = true;
	while(i<in_data->size)
	{
		c = in_data->data[i];
		count = 1;
		while(++i < in_data->size && c == in_data->data[i] && count < UCHAR_MAX) count++;
		if((c==0 && !rle) || (count<rle_len && rle) || (count>rle_len && !rle))
		{
			rle = !rle;
			comp_data->data[n++] = 0;
		}
		if(rle)
		{
			comp_data->data[n++] = count;
			comp_data->data[n++] = c;
		}
		else for(int j=0;j<count;j++) comp_data->data[n++] = c;
	}
	comp_data->size = n;
	write_data(comp_data);
}
void rle_decompress(m_data *comp_data, m_data *dcomp_data)
{
	dcomp_data->data = (unsigned char*)calloc(1048576, sizeof(unsigned char));
	int i = 0, n=0, j;
	unsigned char c;
	bool rle = true;
	while(i<comp_data->size)
	{
		c = comp_data->data[i++];
		if(c == 0) rle = !rle;
		else if(rle) for(j=0,i=i+1;j<c;j++) dcomp_data->data[n++] = comp_data->data[i-1];
		else dcomp_data->data[n++] = c;
	}
	dcomp_data->size = n;
	write_data(dcomp_data);
}
int main(int argc, char **argv)
{
	m_data in_data, out_data;
	int comp = 1;
	if(argc != 4)
	{
		printf("usage: ./binary input_file_name, compressed_file_name, 1 or 0\n");
		exit(0);
	}
	in_data.file_name = strdup(argv[1]);
	out_data.file_name = strdup(argv[2]);
	comp = argc == 4?atoi(argv[3]):1;
	in_data.size = get_file_size(in_data.file_name);
	read_data(&in_data);
	if(comp) rle_compress(&in_data, &out_data);
	else rle_decompress(&in_data, &out_data);
	return 0;
}