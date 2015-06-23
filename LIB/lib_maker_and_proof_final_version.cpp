#include <iostream>
#include <stdio.h>
#include <windows.h>
using namespace std;

#define FIVE_MASK	0x3ff
#define SIX_MASK	0xfff
#define TEMPLATE_CNT	23
//不同位向量在数组中的偏移量
#define off_fiv		0x1486A34	
#define off_six		0x148675B
#define off_sev		0x1485ED0
#define off_eig		0x148452F
#define off_nin		0x147F84C
#define off_ten		0x14711A3
#define off_ele		0x1445DA8
#define off_twe		0x13C41B7
#define off_thi		0x123EDE4
#define off_fou		0xDAF26B
#define off_fif		0

#define MATCH_SIZE	0x1486B27	//3^5+3^6+...+3^15包含所有可能向量匹配结果的数组

const int off_hash[12] = 
{
	MATCH_SIZE,
	off_fiv,		
	off_six,		
	off_sev,		
	off_eig,		
	off_nin,		
	off_ten,		
	off_ele,		
	off_twe,		
	off_thi,		
	off_fou,	
	off_fif	
};

//不同位向量在数组中的偏移量

const int bit_index[15]= 
{ 
	1, 
	3, 
	9, 
	27, 
	81, 
	243, 
	729, 
	2187, 
	6561, 
	19683, 
	59049, 
	177147, 
	531441, 
	1594323, 
	4782969 
};

typedef struct tagTemplate
{
	int	 templ	;
	int	 width	;
	char offset	;

} Template;

const static Template Compare[TEMPLATE_CNT] = 
{
	//长联
	{	0x555,	6,	1,  },
	//成五
	{	0x155,	5,	2,  },
	//活四
	{	0x154,	6,	3,  },
	//冲四
	{	0x55,	5,	4,  },
	{	0x115,	5,	4,  },
	{	0x145,	5,	4,  },
	{	0x151,	5,	4,  },
	{	0x154,	5,	4,  },
	//活三
	{	0x54,	6,	5,  },
	{	0x114,	6,	5,  },
	{	0x144,	6,	5,  },
	{	0x150,	6,	5,  },
	//眠三
	{	0x15,	5,	6,  },
	{	0x150,	5,	6,  },
	{	0x51,	5,	6,  },
	{	0x114,	5,	6,  },
	{	0x45,	5,	6,  },
	{	0x144,	5,	6,  },
	//活二
	{	0x140,	6,	7,  },
	{	0x50,	6,	7,  },
	{	0x14,	6,	7,  },
	{	0x110,	6,	7,  },
	{	0x44,	6,	7,  }
};

char MatchTable[MATCH_SIZE];

static char MatchTemplate( register int bitset, register int width )
{
	register int cx, ax;

	if ( width == 5 )
	{
		for ( cx = 0; cx < TEMPLATE_CNT; cx ++ )
		{
			if ( Compare[cx].width == 5 )
			{
				if ( Compare[cx].templ == bitset )
				{
					return Compare[cx].offset;
				}
				if ( Compare[cx].templ << 1 == bitset )
				{
					//8表示总的模板种类 + 1
					return Compare[cx].offset + 8;
				}
			}
		}
	}
	/*
	 *之所以for循环要加1是为了避免这样一种会由于模板导致的判断错误，如下：
	 *形如 1 0 1 0 1 0 0 1 的向量与形如 0 1 0 1 0 1 的模板进行匹配
	 *如果不加一，那么匹配时程序运行到 比较 1 0 1 0 1 0 和 0 1 0 1 0 1 时认为不相等返回失败
	 */
	int fragfive[11], fragsix[10];

	for ( cx = 0; cx <= width - 5; cx ++ )
	{
		fragfive[cx] = bitset >> (cx * 2) & FIVE_MASK;
	}
	for ( cx = 0; cx <= width - 6; cx ++ )
	{
		fragsix[cx] = bitset >> (cx * 2) & SIX_MASK;
	}
	//匹配成功时若ax为奇数则说明玩家的模板匹配成功，为偶数则说明计算机的模板匹配成功，通过使返回值加上一个常数8来区别
	for ( cx = 0; cx < TEMPLATE_CNT; cx ++ )
	{
		if ( Compare[cx].width == 5 )
		{
			for ( ax = 0; ax <= width - 5; ax ++ )
			{
				if ( Compare[cx].templ == fragfive[ax] )
				{
					return Compare[cx].offset;
				}
				if ( Compare[cx].templ << 1 == fragfive[ax] )
				{
					return Compare[cx].offset + 8;
				}
			}
		}
		else
		{
			for ( ax = 0; ax <= width - 6; ax ++ )
			{
				if ( Compare[cx].templ == fragsix[ax] )
				{
					return Compare[cx].offset;
				}
				if ( Compare[cx].templ << 1 == fragsix[ax] )
				{
					return Compare[cx].offset + 8;
				}
			}
		}
	}
	return 0;
}
static unsigned int BitDecompre( unsigned int input , unsigned int width )
{
	register unsigned int	offset = 0;
	register unsigned int	output = 0;
	while( offset < width )
	{
		output |= (input % 3) << (2 * offset);
		input  /= 3; 
		offset ++;
	}
	return output;
}
/*----------------------------------------------------------------------------------
位压缩实现,要求速度尽可能地快，综合了很多想法后，采用的做法是将每两位二进制压缩成一位三进制
每找到一个为一的位就加上一个该位对应的值,可以再加上一个参数描述向量宽度
实质是一个哈希函数，不过当中还是有很多浪费的空间存在，比如落子点比较少的向量
-----------------------------------------------------------------------------------*/
static unsigned int BitCompress( unsigned int input, unsigned int width )
{
	register unsigned int	offset = 0;
	register unsigned int	output = 0;
	while( offset < width )
	{
		output += bit_index[offset] * ( input >> 2 * offset & 3 );
		offset ++;
	}
	return output;
}
static int LibraryMake( HANDLE hFile )
{
	int ax;
	DWORD	dwWrite;
	memset( MatchTable, 0, MATCH_SIZE * sizeof(char));
	for (int p = 15; p > 4; p --)
	{
		for (int cx = off_hash[p-4]; cx < off_hash[p-5]; cx ++)
		{
			ax = BitDecompre(cx - off_hash[p-4], p);
			MatchTable[cx] = MatchTemplate(ax, p);
		}
	}

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	WriteFile( hFile, MatchTable, MATCH_SIZE * sizeof(char), &dwWrite, NULL );
	return 0;
}
/*----------------------------------------------------------------------------------*/
//输出二进制
static void BinaryPrint(unsigned int cx)
{
	if (cx == 0)
	{
		return;
	}
	cout<<(cx&1)<<" ";
	BinaryPrint(cx>>1);
}
/*----------------------------------------------------------------------------------*/
int main()
{
	HANDLE hFile = CreateFile(TEXT("Dictionary.DAT"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	LibraryMake(hFile);
	CloseHandle(hFile);
	/*-----------------------------------------------------------------------
	int ax, bx;
	for (int p = 15; p > 4; p --)
	{
		for (int cx = off_hash[p-4]; cx < off_hash[p-5]; cx ++)
		{
			ax = BitDecompre(cx - off_hash[p-4], p);
			bx = BitCompress(ax, p);
			if (cx - off_hash[p-4] != bx)
			{
				cout<<"ERROR OCCOUR!"<<endl;
			}
		}
	}
	cout<<"NO ERROR OCCOUR!"<<endl;
	----------------------------------------------------------------------------
	unsigned int cx, bx;
	cx = 0xaa400;
	bx = MatchTemplate(cx, 15);
	cout<<bx<<endl;
	----------------------------------------------------------------------------
	000000000010101010010000000000
	000000000001101010100000000000
	----------------------------------------------------------------------------
	cout<<"-------------------分割线--------------------"<<endl;
	cx = 0x6a800;
	bx = MatchTemplate(cx,15);
	cout<<bx<<endl;
	-----------------------------------------------------------------------------*/
	return 0;
}