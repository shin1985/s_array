//============================================================================
// Name        : s_array.h
// Author      : Shingo.Akiyoshi
// Version     : 0.1(2008)
// Copyright   : master@akishin.net�܂ł��₢���킹���������B
// Description : SuffixArray in C, Ansi-style => �C���^�[�t�F�[�X����C++�֕ύX(2008.2.14)
// Memo        : �������팸�̂��߁AS-JIS�O��̎������Ă܂��BUTF-8�ɂ��Ȃ����ƁI
//============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "szsort.h"



class SN_SUFFIX_ARRAY{
private:
	char array_name[128];
	unsigned int array_num;
	unsigned int *backet;
public:
	char *map_data;
	unsigned int *lcp;
	const char *DIR_NGRAM;
	unsigned int real_len;
public:
	typedef struct{
		unsigned int high;
		unsigned int low;
		unsigned int pos;
	}BINARY_SEARCH_POINTER;
	typedef struct{
		unsigned int count_forward;//���O�̏�Ԃ̏o����
		unsigned int count_back;//���O�̏�Ԃ���w�肵�����̏�Ԃ֍s���ꍇ�̏o����
	}COUNT_RESULT;
	typedef struct{
		unsigned int count;
		char *word;
	}NGRAM_COUNT;
public:
	BINARY_SEARCH_POINTER binary_search(const char *back_c, BINARY_SEARCH_POINTER bsp);
public:
	//�R���X�g���N�^
	SN_SUFFIX_ARRAY();
	//~SN_SUFFIX_ARRAY();
	void dir_path_change(const char *data_dir_path);
	//�V�K��Array���\�z����B���������Array�̐���Ԃ��B
	unsigned int mk_array(const char *array_name, char *text, int len);
	//�\�z�ς݂�Array���t�@�C������ǂݍ��ށB���������Array����Ԃ��B
	unsigned int fileread(const char *array_name);
	//�o���񐔂��J�E���g
	unsigned int get_count(const char *keyword);
	//�P��bi-gram�̃J�E���g
	COUNT_RESULT get_bigram_count(const char *forward_word, const char *back_word);
	//�w�肵��n�����̃J�E���g���o�͂���(Debug)
	void show_ngram(int n);
	//LCP���\�z����B
	unsigned int *build_lcp(unsigned int *backet, unsigned int array_num);
	//������`�F�b�N�BASCII�������p���ȁA����R�[�h�̏ꍇtrue��Ԃ��B
	bool check_asii(const char *cp);
	char *read_text(const char *file_name);
	//��v���������񒷂�Ԃ�
	unsigned int get_common_prefix_length(unsigned int *idx_p1, unsigned int *idx_p2);
private:
	int compmi(const char *m1, const unsigned int *m2, unsigned int *pos);
	const unsigned int *bsearch(const char *_key, const unsigned int *_Base, size_t begin, size_t end, size_t pos);
	char *str_ncpy(char *_Dest, const char *_Source, size_t _Count);
};

int isZenkaku(unsigned char c){
	return (((c >= 0x81) && (c <= 0x9f)) || ((c >= 0xe0) && (c <= 0xfc)));
}

bool SN_SUFFIX_ARRAY::check_asii(const char *cp){
	/*unsigned char * c = (unsigned char *)cp;
	//printf("code=%x\n", *c);
	if( (*c>=0x00 && *c<=0x1F) || *c==0x7F){//����R�[�h
		return true;
	}else if(*c>=0x20 && *c<=0x7E){//ASCII����
		return true;
	}else if(*c>=0xA1 && *c<=0xDF){//���p����(�g�p�֎~)
		return true;
	}
	return false;*/
	return !(isZenkaku(*cp));
}


SN_SUFFIX_ARRAY::SN_SUFFIX_ARRAY(){
	static const char DEFAULT_PATH[]= "data";
	this->array_num = 0;
	this->DIR_NGRAM = DEFAULT_PATH;
}

/*SN_SUFFIX_ARRAY::~SN_SUFFIX_ARRAY(){
	//delete(this->backet);
	//delete(this->map_data);
	//�����߂�ǂ��BGC���錾��ɈڐA������(TAT)
}*/


void SN_SUFFIX_ARRAY::dir_path_change(const char *data_dir_path){
	this->DIR_NGRAM = data_dir_path;
}

struct GetLetter {
  unsigned operator()(const unsigned int &backet, size_t depth) {
	  return (unsigned char)StringSort::mapdata[backet + depth];
  }
};


unsigned int SN_SUFFIX_ARRAY::mk_array(const char *array_name, char *text, int len){
	char file_name[128];
	FILE *fp;
	unsigned int count[UCHAR_MAX];
	unsigned int offset[UCHAR_MAX];
	int i;
	//unsigned int *backet;

	this->map_data = text;
	this->real_len = strlen(text);

	//��������Array�̖��O��ݒ�
	strcpy((char *)this->array_name, array_name);
	//���R�[�p�X�ۑ�
	sprintf(file_name, "%s/%s.dat", this->DIR_NGRAM, this->array_name);
	fp = fopen(file_name, "wb");
	fwrite(text, sizeof(char), len, fp);
	fclose(fp);
	//�S��0�ɂ���
	for(int i=0;i<UCHAR_MAX;i++){ count[i]=0;}
	//�o���񐔂𐔂��Ȃ��甼�p�E�S�p���f�������o���B
	sprintf(file_name, "%s/%s.idx", this->DIR_NGRAM, this->array_name);
	fp = fopen(file_name, "wb");
	for(i=0,this->array_num=0; i<(int)len; i++){
		if(text[i]=='\n' || text[i]=='\r' || text[i]==' ' || text[i]=='\t')
			continue;
		if(ispunct((unsigned char)text[i])!=0)
			continue;
		fwrite(&i, sizeof(unsigned int), 1, fp);
		count[(unsigned char)text[i]]++;
		this->array_num++;
		if(!check_asii((char *)&text[i]))
			i++;
	}
	fclose(fp);
	printf("�O��������\n");
	//�J�n�ʒu����
	offset[0]=0;
	for(i=1; i<UCHAR_MAX; i++){
		offset[i]=offset[i-1]+count[i-1];
	}
	//RADIX�\�[�g�J�n
	printf("RADIX�\�[�g�J�n\n");
	this->backet = (unsigned int *)malloc(sizeof(unsigned int)*this->array_num);
	printf("Array�T�C�Y:%d\n", this->array_num);
	if(this->backet==NULL){
		perror("error");
		exit(EXIT_FAILURE);
	}
	fp = fopen(file_name, "rb");

	for(int i=0; i<this->array_num; i++){
		unsigned int buf;
		fread(&buf, sizeof(unsigned int), 1, fp);
		//printf("%d\n", buf);
		this->backet[offset[(unsigned char)(text[buf])]]=buf;
		offset[(unsigned char)text[buf]]++;
	}
	fclose(fp);
	printf("RADIX�\�[�g����\n");

	StringSort::Sort(this->map_data, this->backet, this->backet + this->array_num, 0, GetLetter());
	//StringSort(this->backet, this->backet + this->array_num, 0);
	printf("multikey_quick_sort����\n");

	fp = fopen(file_name, "wb");
	fwrite(&this->array_num, sizeof(unsigned int), 1, fp);
	fwrite(&this->array_name, sizeof(char), 128, fp);
	fwrite(this->backet, sizeof(unsigned int), this->array_num, fp);
	fclose(fp);

	//LPC�v�Z
	printf("LCP�v�Z��\n");
	this->lcp = this->build_lcp(this->backet, this->array_num);
	printf("LCP�v�Z����\n");
	//LCP�ۑ�
	sprintf(file_name, "%s/%s.lcp", this->DIR_NGRAM, this->array_name);
	fp = fopen(file_name, "wb");
	fwrite(this->lcp, sizeof(unsigned int), this->array_num, fp);
	fclose(fp);

	return this->array_num;
}

unsigned int *SN_SUFFIX_ARRAY::build_lcp(unsigned int *backet, unsigned int array_num){
	unsigned int *lcp = (unsigned int *)malloc(sizeof(unsigned int)*array_num);
	for(unsigned int i=0; i<=this->array_num-2; i++){
		lcp[i] = this->get_common_prefix_length(&backet[i],&backet[i+1]); 
	}
	lcp[this->array_num-1]=0;
	return lcp;
}

unsigned int SN_SUFFIX_ARRAY::get_common_prefix_length(unsigned int *idx_p1, unsigned int *idx_p2){
	unsigned int res=0;
	unsigned int idx_p1_len = this->real_len = *idx_p1;
	unsigned int idx_p2_len = this->real_len - *idx_p2;
	for(res = 0; res < idx_p1_len && res < idx_p2_len; res++){
		if(this->map_data[*idx_p1+res]!=this->map_data[*idx_p2+res])
			break;
	}
	return res;
}

SN_SUFFIX_ARRAY::BINARY_SEARCH_POINTER SN_SUFFIX_ARRAY::binary_search(const char *back_c, BINARY_SEARCH_POINTER bsp){ //backup
	const unsigned int *res = this->bsearch(back_c, this->backet, bsp.high, bsp.low, bsp.pos);
	if(res==NULL){
		bsp.high = 0;
		bsp.low = 0;
		return bsp;
	}
	unsigned int len = strlen(back_c);
	//�������
	int i;
	for(i=(res - this->backet)-1; i>=bsp.high; i--){
		if(this->lcp[i]+bsp.pos<len)
			break;
	}
	bsp.high = i+1;
	//��������
	for(i=(res - this->backet); i<bsp.low; i++){
		if(this->lcp[i]+bsp.pos<len)
			break;

	}
	bsp.low = i+1;
	return bsp;
}

int SN_SUFFIX_ARRAY::compmi(const char *m1, const unsigned int *m2, unsigned int *pos){
   return strncmp(m1, &this->map_data[*m2+*pos], strlen(m1));
}



const unsigned int *SN_SUFFIX_ARRAY::bsearch(const char *_key, const unsigned int *_Base, size_t begin, size_t end, size_t pos){
	if(begin >= end){
		return NULL;
	}
	int ret = this->compmi(_key, &_Base[begin + (end-begin)/2], (unsigned int *)&pos);
	if(ret==0)
		return &_Base[begin + (end-begin)/2];
	else if(ret>0)
		return this->bsearch(_key, _Base, begin+(end-begin)/2+1, end, pos);
	else
		return this->bsearch(_key, _Base, begin, begin + (end-begin)/2, pos); 

}

unsigned int SN_SUFFIX_ARRAY::get_count(const char *keyword){
	unsigned int res;
	SN_SUFFIX_ARRAY::BINARY_SEARCH_POINTER bsp;
	bsp.high = 0;
	bsp.low = this->array_num;
	bsp.pos = 0;
	//bsp.len = strlen(keyword);

	bsp = this->binary_search(keyword, bsp);
	res = bsp.low - bsp.high;
	if(res <= 0)
		res = 0;

	return res;

}


SN_SUFFIX_ARRAY::COUNT_RESULT SN_SUFFIX_ARRAY::get_bigram_count(const char *forward_word, const char *back_word){
	SN_SUFFIX_ARRAY::COUNT_RESULT res;
	SN_SUFFIX_ARRAY::BINARY_SEARCH_POINTER bsp;
	bsp.high = 0;
	bsp.low = this->array_num;
	bsp.pos = 0;
	//bsp.len = strlen(forward_word);

	//�O�̕������v�Z
	bsp = this->binary_search(forward_word, bsp);
	res.count_forward = bsp.low - bsp.high;

	if(res.count_forward<=0){
		res.count_forward = 0;
		res.count_back = 0;
	}else{
		bsp.pos=strlen(forward_word);
		//bsp.len += strlen(back_word);
		//���̕������v�Z
		bsp = this->binary_search(back_word, bsp);
		res.count_back = bsp.low - bsp.high;
	}

	return res;
}

char *SN_SUFFIX_ARRAY::str_ncpy(char *_Dest, const char *_Source, size_t _Count){
	for(int i=0; i<_Count; i++){
		if(this->check_asii(&_Source[i])){
			_Dest[i]=_Source[i];
		}else{
			_Dest[i]=_Source[i];
			i++;
			_Dest[i]=_Source[i];
			_Count++;
		}
	}
	return _Dest;
}

void SN_SUFFIX_ARRAY::show_ngram(int n){
	char word[128]="";
	unsigned int count=0;
	for(unsigned int i=0; i<this->array_num; i++){
		if(strncmp(word, &this->map_data[this->backet[i]], n)!=0){
			printf("%s,%ld\n", word, count);
			count = 0;
			this->str_ncpy(word, &this->map_data[this->backet[i]], n);
			//strncpy(word, &this->map_data[this->backet[i]], n);
		}
		count++;
	}
}

char *SN_SUFFIX_ARRAY::read_text(const char *file_name){

	fpos_t fsize = 0;
	printf("%s���J���܂�\n", file_name);
	FILE *fp = fopen(file_name, "rb");
	if(fp == NULL){
		perror("���̓t�@�C�����J���܂���");
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &fsize);
	printf("size=%d\n", fsize);
	this->map_data = (char *)malloc(sizeof(char)*fsize);
	fseek(fp,0,SEEK_SET);
	fread((char *)this->map_data, sizeof(char), fsize, fp);
	fclose(fp);
	return this->map_data;

}


unsigned int SN_SUFFIX_ARRAY::fileread(const char *array_name){
	char file_name[128];
	FILE *fp;
	strcpy((char *)this->array_name, array_name);
	sprintf(file_name, "%s/%s.dat", DIR_NGRAM, this->array_name);
	this->map_data = read_text(file_name);
	sprintf(file_name, "%s/%s.idx", DIR_NGRAM, this->array_name);
	fp = fopen(file_name, "rb");
	if(fp==NULL){
		perror("�C���f�L�V���O�f�[�^���J���܂���\n");
		exit(EXIT_FAILURE);
	}
	fread(&this->array_num, sizeof(unsigned int), 1, fp);
	fread((char *)this->array_name, sizeof(char), 128, fp);
	this->backet = (unsigned int *)malloc(sizeof(unsigned int)*this->array_num);
	fread(this->backet, sizeof(unsigned int), this->array_num, fp);
	fclose(fp);
	this->real_len = strlen(this->map_data);

	//LCP�v�Z
	this->lcp = (unsigned int *)malloc(sizeof(unsigned int)*array_num);
	sprintf(file_name, "%s/%s.lcp", this->DIR_NGRAM, this->array_name);
	fp = fopen(file_name, "rb");
	if(fp==NULL){
		perror("LCP�f�[�^���J���܂���\n");
		exit(EXIT_FAILURE);
	}
	fread(this->lcp, sizeof(unsigned int), this->array_num, fp);
	fclose(fp);

	return this->array_num;
}
