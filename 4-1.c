/*	
 *		Huff Encode and Decode
 *
 *	      WIN7x64   VC6.0
 *
 *	���ļ����й�����ѹ���� �ٽ��н�ѹ��
 *	
 *	֧�������ʽ�ļ���ԭ�ļ���С��С��4GB ��ѹ�����������ļ���ʽ"*.hfc"ͨ���ļ�ͷ�����������
 *	
 *	���ı�,��ִ�г���λͼ�ļ����зǳ��õ�ѹ��Ч�� 
 *
 *	���Ѿ����ýϺ�ѹ����ʽ��ͼƬ����Ƶ����Ƶ���ļ���ѹ���ʲ�����
 *
 *	///
 *	
 *	2014 11 29   	
 *
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct 
{
	unsigned int weight;
	unsigned int P;
	unsigned int L;
	unsigned int R;
	unsigned int data;
}HF,*HF_Tree;//�������������ṹ��
typedef struct  
{
	unsigned int *table_num;
	unsigned char *data;
}data_num_table;//Ȩֵ��ṹ
#pragma pack(push)
#pragma pack(1)
typedef struct  
{
	unsigned char head[4];	//magic number
	unsigned char check;	//У��λ //����
	unsigned char zero;	//ĩβ�������λ //���� //���� 
	unsigned char table_start;	//�����ʼ
	unsigned char key_flag;     //���ܱ�־λ //���� 
	unsigned int count;	//�����ַ���Ա����
	unsigned int code_len;		//�����С
	unsigned int len;		//Դ���С
	unsigned int code_start;    //���뿪ʼ
}code_file;		//�ļ�ͷ�ṹ��
#pragma pack(pop)
int encoder(char *filename);  //������
int read_src(unsigned int *table, char *filename);//���ļ���ͳ��Ȩֵ
void find_table(unsigned int *table, data_num_table *new_table, unsigned int *count);//��ЧȨֵ������
void create_huff(data_num_table new_table, HF_Tree ht, unsigned int count);//������������
int select_min(HF_Tree ht, unsigned int n, unsigned int *min_1, unsigned int *min_2);//����С����Ȩֵ
unsigned char **huff_code(HF_Tree ht, unsigned int count);//����
int encode_file(const char *fillename, data_num_table new_table, const unsigned char **code_table, unsigned int count);//���뵽�ļ�
int decoder(char *filename);//������
int read_dst(data_num_table *table, char *filename);//���ļ�����ԭȨֵ��
int decode_file(char *filename, HF_Tree ht);//��ԭ�ļ�
int check_head();//����ļ�ͷ
unsigned int max_buf_fun(unsigned int len);//���㻺������С
int calc_size(unsigned int *k, FILE *fp, unsigned int max_buf, unsigned char *buffer, unsigned int len);//����Ӧ��ȡ���ݿ��С
unsigned int calc_step(unsigned int len);////�������ˢ�²���
///////////////////////// //////////////

code_file format = {"HFC", 0, 0, 0, 0, 0, 0}; 
long runtime;		// ����ʱ�����

//////////////////////////////////////ȫ�ֱ���


int main(int argc, char *argv[])
{
	char filename[120], ch;

	printf("\n\t\t\t\tHuff Encode and Decode!");
	printf("\n\n->Decode or Encode?(D or E)?");
	ch = getch();
	if (argc >= 2)
	{
		if (ch == 'D' || ch == 'd')
		{
			decoder(argv[1]);
		}		
		else
		{
			encoder(argv[1]);
		}
	}
	else
	{
		if (ch == 'D' || ch == 'd')
		{
			printf("\n->Decode filename:");
			scanf("%119[^\n]s", filename);	
			decoder(filename);
		}		
		else
		{
			printf("\n->Encode filename:");
			scanf("%119[^\n]s", filename);
			encoder(filename);
		}
	}
	getch();
	return 0;
}

int encoder(char *filename)
{
	data_num_table new_table;
	unsigned int count = 0;
	unsigned int table[257];
	HF_Tree ht;
	unsigned char **code_table = NULL;

	memset(table, 0, sizeof(table));
	read_src(table, filename); //��ԭ�ļ���ͳ����ӦASCIIֵ����
	find_table(table, &new_table, &count);	//	�ҵ���ЧȨֵ
	if(!(ht = (HF_Tree)malloc((count*2) * sizeof(HF))))
	{
		printf("\n->No Memory!");
		getch();
		exit(1);
	}
	runtime = clock(); //��ǿ�ʼʱ��
	create_huff(new_table, ht, count);	//������������
	code_table = huff_code(ht, count);  //���й���������
	encode_file(filename, new_table, code_table, count);
	
	return 0;
}
int read_src(unsigned int *table, char *filename)
{
	unsigned int  len, max_buf, k, j, m;
	unsigned char *buffer;
	FILE *fp_src;
	
	if(!(fp_src = fopen(filename, "rb")))
	{
		printf("\n->Error!File:\"%s\" Not Found!", filename);
		getch();
		exit(1);
	}
	else
		printf("\n->Open File Done!");
	fseek(fp_src, 0, SEEK_END);
	len = ftell(fp_src);
	rewind(fp_src);
	format.len = len;
	if (len == 0)
	{
		printf("\n->Error!Empty File or File too big!");
		getch();
		exit(1);
	}
	printf("\n->Creating Table...");
	max_buf = max_buf_fun(len);
	buffer = (unsigned char *)malloc(max_buf+1);
	k = 0;
	do
	{
		m = k;
		calc_size(&k, fp_src, max_buf, buffer, len);
		m = k - m;
		for (j = 0; j < m; j++)
		{
			++table[buffer[j]];
		}
	}while (k < len);
	fclose(fp_src);
	return 0;
}
void find_table(unsigned int *table, data_num_table *new_table, unsigned int *count)
{
	unsigned int i, j;
	
	for (j = 0, i = 0; j <= 255; j++)
	{
		if (table[j] != 0)
		{
			i++;
		}
	}
	*count = i;
	new_table->data = (unsigned  char*)malloc((i+1)*sizeof(unsigned char));
	new_table->table_num = (unsigned int *)malloc((i+1)*sizeof(unsigned int));
	if (!new_table->data || !new_table->table_num)
	{
		printf("\n->Error!Create Table Failed!");
		getch();
		exit(1);
	}
	else
	{
		printf("\n->Create Table Done!");
	}
	for (j = 0, i = 1; j <= 255; j++)
	{
		if (table[j] != 0)
		{
			*(new_table->table_num+i) = table[j];	//����Ȩֵ
			*(new_table->data+i) = (unsigned char)j;			//��������
			 i++;
		}
	}
}
void create_huff(data_num_table new_table, HF_Tree ht, unsigned int count)
{
	unsigned int i, m, min_1 = 0, min_2 = 0;
	
	for (i = 1; i <= count; i++)
	{
		ht[i].weight = *(new_table.table_num+i);
		ht[i].L = 0;
		ht[i].R = 0;
		ht[i].P = 0;
		ht[i].data = *(new_table.data+i);
	}						//��ʼ����������
	m = 2 * count - 1;
	for (i = count + 1; i <= m; i++)
	{
		ht[i].weight = 0;
		ht[i].L = 0;
		ht[i].R = 0;
		ht[i].P = 0;
		ht[i].data = 0;
	}
	for (i = count + 1; i <= m; i++)
	{
		select_min(ht, i - 1, &min_1, &min_2);
		ht[i].weight = ht[min_1].weight + ht[min_2].weight;
		ht[min_1].P = i;
		ht[min_2].P = i;
		ht[i].L = min_1;
		ht[i].R = min_2;
	}
	printf("\n->Create HuffTree Done!");
	//���������������
}

int select_min(HF_Tree ht, unsigned int n, unsigned int *min_1, unsigned int *min_2)
{
	unsigned int swap = 0xffffffff, swap_last = swap; 
	unsigned int i;

	
	for (i = 1; i <= n; i++)
	{
		if (ht[i].P == 0)
		{
			if (swap > ht[i].weight)
			{
				swap_last = swap;
				swap = ht[i].weight;
				*min_2 = *min_1;
				*min_1 = i;
			}
			else if(swap_last > ht[i].weight)
			{
				swap_last = ht[i].weight;
				*min_2 = i;
			}
		}
	}
	return 0;
}
unsigned char **huff_code(HF_Tree ht,unsigned int count)
{
	unsigned char *buffer;
	unsigned int i, start, c, p;
	unsigned char **code_table; 

	code_table = (unsigned char **)malloc((count + 1)*sizeof(unsigned char *));
	buffer = (unsigned char *)malloc(count + 1);
	memset(buffer, 0, count + 1);
	for (i = 1; i <= count; i++)
	{
		start = count;
		c = i;
		p = ht[i].P;
		while (p != 0)
		{
			--start;
			if (ht[p].L == c)
			{
				buffer[start] = '0';
			}
			else
			{
				buffer[start] = '1';
			}
			c = p;
			p = ht[p].P;
		}
		code_table[i] = (unsigned char *)malloc((count - start)*sizeof(unsigned char));
		if (!code_table[i])
		{
			printf("\n->Error!No Memory!");
			getch();
			exit(1);
		}
		strcpy(code_table[i], &buffer[start]);
	}
	free(buffer);
	buffer = NULL;
	printf("\n->Code Done!");
	return code_table;
}
int encode_file(const char *filename,  data_num_table new_table, const unsigned char **code_table, unsigned int count)
{
	FILE *fp_src;
	FILE *fp_dst;
	unsigned int j, len, max_buf, k, new_len, p, m, q, n, x, i, step;
	char dst_file_name[125];
	unsigned char *buffer_file, *buffer_hex;	// ���ļ����ݿ黺����  д������ѹ�����ݿ黺����
	unsigned char sum = 0, zero = 0;
	const unsigned char *code_point; //�����ָ�� 
	

	strcpy(dst_file_name, filename);
	strcat(dst_file_name, ".hfc");  //�������ļ��� *.*.hfc
	if (!(fp_dst = fopen(dst_file_name, "wb")))
	{
		printf("\n-->Create Encode File failed!");
		getch();
		exit(1);
	}
	if (!(fp_src = fopen(filename, "rb")))
	{
		printf("\n-->Open File failed!");
		getch();
		exit(1);
	}
	fseek(fp_src, 0, SEEK_END);
	len = ftell(fp_src);
	rewind(fp_src);
	max_buf = max_buf_fun(len);	//���㻺������С
	buffer_file = (unsigned char *)malloc(max_buf+1);
	buffer_hex = (unsigned char *)malloc(max_buf+1);
	if(!buffer_file || !buffer_hex)
	{
		printf("\n->Error!No Memory!");
		getch();
		exit(1);
	}
	else
	{
		printf("\n->Create Buffer Done!");
	}
	memset(buffer_file, 0, sizeof(buffer_file));
 	fseek(fp_dst, sizeof(code_file), SEEK_SET);
	fwrite(new_table.table_num+1, sizeof(unsigned int)*count, 1, fp_dst);//д��Ȩֵ��
	fwrite(new_table.data+1, sizeof(unsigned char)*count, 1, fp_dst);//д���ַ���
	format.code_start = sizeof(code_file) + count * (sizeof(unsigned int)+sizeof(unsigned char));
	k = 0;
	m = 0;
	q = 0;
	x = 0;
	printf("\n");
	step = calc_step(len);//�������ˢ�²���
	fseek(fp_dst, format.code_start, SEEK_SET);
	do
	{
		i = k;
		calc_size(&k, fp_src, max_buf, buffer_file, len);
		i = k - i;
		for (j = 0; j < i; j++)	//���ƴ��ļ������ݿ�����ж�����
		{
			for (p = 1; buffer_file[j] != *(new_table.data+p); p++);//��ѯ�����
			code_point = code_table[p];
			n = 0;
			while(code_point[n])//д���������ݻ�����
			{
				if((code_point[n] - 48) != 0)//�����0�Ļ��Ͳ��ü�����
				{
					sum += ((code_point[n] - 48)<<(7 - q%8));
				}	
				if ((q+1) % 8 == 0)//ÿ��8������ ��sumֵд���������������
				{
					buffer_hex[m] = sum;
					sum = 0;
					++m;//��������ָ�롱
					if (m  == max_buf)//��������������ʱ������������д���ļ�
					{
						fwrite(buffer_hex, max_buf, 1, fp_dst);
						m = 0;//ˢ�»�����
					}
				}
				q++;//�ܱ������������
				n++;//ÿ���������������
			}
			if (x%step == 0)
			{
				printf("\r->Encoding... %.2lf%%", (double long)x/len * 100);
			}
			++x;//���ȼ���
		}
	}while(k < len);
	printf("\r->Encoded ... 100.00%%");
	format.code_len = q;
	fwrite(buffer_hex, m, 1, fp_dst);
	fwrite(&sum, 1, 1, fp_dst);//�������8λ������д���ļ�
	format.count = count;
	format.table_start = sizeof(code_file);
	rewind(fp_dst);
	fwrite(&format, sizeof(code_file), 1,fp_dst);//д���ļ�ͷ�ṹ��    //������̫��  //ò�ƻ���
	fseek(fp_dst, 0, SEEK_END);
	new_len = ftell(fp_dst);
	free(buffer_hex);
	free(buffer_file);
	fclose(fp_src);
	fclose(fp_dst);
	printf("\n->Encode Done!");	//���ɱ���
	printf("\n->Create Report:\n-----------------------------\n-----------------------------");
	printf("\n->Old File Size:   %lu KB", format.len/1024);
	printf("\n->New File Size:   %lu KB", new_len/1024);
	printf("\n->Compare:         %.2Lf%%", (double long)new_len/format.len * 100);
	printf("\n->Take Your Time:  %lf s", (double)(clock() - runtime)/CLOCKS_PER_SEC);
	printf("\n-----------------------------\n-----------------------------\n->Press Any Key to Exit!");
	return 0;
}

int decoder(char *filename)
{
	data_num_table table;
	HF_Tree ht;

	read_dst(&table, filename);
	if(!(ht = (HF_Tree)malloc((format.count * 2) * sizeof(HF))))
	{
		printf("\n->Error!No Memory!");
		getch();
		exit(1);
	}
	runtime = clock(); //��ǿ�ʼʱ��
	create_huff(table, ht, format.count);	//������������
	decode_file(filename, ht);

	return 0;
}

int read_dst(data_num_table *table, char *filename)
{
	FILE *fp_endcoded;
	unsigned char magic_head[5]={0};
	unsigned int len;

	if(!(fp_endcoded = fopen(filename, "rb")))
	{
		printf("\n->Error!File Not Found!");
		getch();
		exit(1);
	}
	else
		printf("\n->Open File Done!");
	len = strlen(filename) - 4;
	fread(magic_head, 4, 1, fp_endcoded);
	if (strcmp(magic_head, "HFC") != 0)
	{
		printf("\n->Error!Unkonw Filetype!");
		getch();
		exit(1);
	}
	rewind(fp_endcoded);
	fread(&format, sizeof(code_file), 1, fp_endcoded);
	check_head();		//�ļ�ͷУ��
	table->table_num = (unsigned int *)malloc((format.count+2)*sizeof(unsigned int));
	table->data = (unsigned char *)malloc((format.count+2)*sizeof(unsigned char));
	if (!table->data || !table->table_num)
	{
		printf("\n->Error!Create Table Failed!");
		getch();
		exit(1);
	}
	fseek(fp_endcoded, format.table_start, SEEK_SET);
	if(!fread((table->table_num+1), sizeof(unsigned int)*format.count, 1,fp_endcoded))
	{
		printf("\n->ERROR!File Exception!");
		getch();
		exit(1);
	}
	if (!fread((table->data+1), sizeof(unsigned char)*format.count, 1,fp_endcoded))
		{
			printf("\n->ERROR!File Exception!");
			getch();
			exit(1);
	}
	printf("\n->Create Table Done!");
	fclose(fp_endcoded);
	return 0;
}
int decode_file(char *filename, HF_Tree ht)
{
	unsigned int i, j, p, len, old_len, step;
	//	unsigned int max_buf, k, m;
	FILE *fp_encode;
	FILE *fp_decode;
	//	unsigned char *buffer;
	unsigned char flag = 0, temp_bit, tmp;
	char dst_filename[125];

	if(!(fp_encode = fopen(filename, "rb")))
	{
		printf("\n->Error!File Not Found!");
		getch();
		exit(1);
	}
	len = strlen(filename) - 4;
	memset(dst_filename, 0, sizeof(dst_filename));
	strncpy(dst_filename, filename, len);
	if(!(fp_decode = fopen(dst_filename, "wb")))
	{
		printf("\n->Create File Failed!");
		getch();
		exit(1);
	}
// 	fseek(fp_encode, 0, SEEK_END);
// 	len = ftell(fp_encode) - format.code_start;
// 	max_buf = max_buf_fun(len);
// 	buffer = (unsigned char *)malloc(max_buf+1);
// 	k = 0;
// 	j = 0;
// 	fseek(fp_encode, format.code_start, SEEK_SET);
// 	do
// 	{
// 		m = k;
// 		calc_size(&k, fp_encode, max_buf, buffer, len);
// 		m = k - m;
// 		for (i = 0; i < m && j < format.code_len; )
// 		{
// 			p = 2 * format.count - 1;//���ڵ�
// 			while(ht[p].L != 0 && ht[p].R != 0 && i < m)
// 			{
// 				temp_bit = buffer[i] << j%8;
// 				temp_bit = temp_bit >> 7;
// 				if (j%8 == 0)
// 				{
// 					++i;
// 				}
// 				if(temp_bit == 0) 
// 					p = ht[p].L;
// 				if(temp_bit == 1) 
// 					p = ht[p].R;
// 				j++;
// 			}
// 			if ()
// 			{
// 			}
// 			fwrite(&ht[p].data, 1, 1, fp_decode);	
// 		}	
// 	}while (k < len);
	j = 0;
	step = calc_step(format.len);//�������ˢ�²���
	fseek(fp_encode, format.code_start, SEEK_SET);
	printf("\n");
	for(i = 0; i < format.len && j < format.code_len; i++)
	{ 
		p = 2 * format.count - 1;
		while(ht[p].L != 0 && ht[p].R != 0)
		{
			if (j % 8 == 0)
			{
				if(!fread(&tmp, 1, 1, fp_encode))
				{
					printf("\n->ERROR!File Exception!");
					getch();
					exit(1);
				}
			}
			temp_bit = tmp << j%8;
			temp_bit = temp_bit >> 7;
			if(temp_bit == 0) 
				p = ht[p].L;
			if(temp_bit == 1) 
				p = ht[p].R;
			j++;
		}
		fwrite(&ht[p].data, 1, 1, fp_decode);
		if (i%step== 0)
		{
			printf("\r->Decoding... %.2lf%%", (double long)i/format.len * 100);
		}
	}
	printf("\r->Decoded...  100.00 %%");
	fseek(fp_encode, 0, SEEK_END);
	old_len = ftell(fp_encode);
	fclose(fp_decode);
	fclose(fp_encode);
	printf("\n->Decode Done!");
	printf("\n->Create Report:\n-----------------------------\n-----------------------------");
	printf("\n->Old File Size:   %lu KB", old_len/1024);
	printf("\n->New File Size:   %lu KB", format.len/1024);
	printf("\n->Compare:         %.2Lf%%", (double long)format.len/old_len * 100);
	printf("\n->Take Your Time:  %lf s", (double)(clock() - runtime)/CLOCKS_PER_SEC);
	printf("\n-----------------------------\n-----------------------------\n->Press Any Key to Exit!");
	return 0;
}
int check_head()
{
	if (!format.len || (format.table_start - sizeof(code_file)) || !format.code_len || !format.count || (format.code_start - sizeof(code_file) - format.count * (sizeof(unsigned char)+sizeof(unsigned int))))
	{
		printf("\n->ERROR!Check File Head Faided!");
		getch();
		exit(1);
	}
	printf("\n->Check Head Done!");
	return 0;
}
unsigned int max_buf_fun(unsigned int len)
{
	unsigned int max_buf;
	if (len < 1024 * 512)
	{
		max_buf = 1024 * 512;
	}
	else if (len < 1024 * 1024)
	{
		max_buf = 1024 * 1024;
	}
	else
	{
		max_buf = 1024 * 1024 * 8;
	}
	return max_buf;
	//��������С ���8MB  ��С512KB
}
int calc_size(unsigned int *k, FILE *fp, unsigned int max_buf, unsigned char *buffer, unsigned int len)
{
	if (len - *k > max_buf)//�ж�Ӧ�������ݴ�С
	{
		if(!fread(buffer, max_buf, 1, fp))
		{
			printf("\n->ERROR!File Exception!");
			getch();
			exit(1);
		}
		*k = ftell(fp);
	}
	else
	{
		fread(buffer, len - *k, 1, fp);
		*k = len;
	}
	return 0;
}
unsigned int calc_step(unsigned int len)
{
	if (len < 0xA0)
	{
		return  1;
	}
	else if(len < 0X3e8)
	{
		return len / 10; 
	}
	else if (len < 0XA00000)
	{
		return len / 100;
	}
	else if (len < 0X64000000)
	{
		return len / 500;
	}
	else if (len < 0x3e8000000)
	{
		return len / 1000;
	}
	else
	{
		return len / 2000;
	}
}
