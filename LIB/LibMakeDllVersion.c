#include <windows.h>
#define FIVE_MASK	0x3ff
#define SIX_MASK	0xfff
#define TEMPLATE_CNT	23
#define OFFSET_FIV	0x1486A34	
#define OFFSET_SIX	0x148675B
#define OFFSET_SEV	0x1485ED0
#define OFFSET_EIG	0x148452F
#define OFFSET_NIN	0x147F84C
#define OFFSET_TEN	0x14711A3
#define OFFSET_ELE	0x1445DA8
#define OFFSET_TWE	0x13C41B7
#define OFFSET_THT	0x123EDE4
#define OFFSET_FOT	0xDAF26B
#define OFFSET_FIT	0
#define MATCH_SIZE	0x1486B27	//3^5+3^6+...+3^15�������п�������ƥ����������
//��ͬλ�����������е�ƫ����

typedef struct tagTemplate
{
	int	 templ	;
	int	 width	;
	BYTE	offset	;

} Template;

const static Template Compare[TEMPLATE_CNT] = 
{
	//����
	{	0x555,	6,	1,  },
	//����
	{	0x155,	5,	2,  },
	//����
	{	0x154,	6,	3,  },
	//����
	{	0x55,	5,	4,  },
	{	0x115,	5,	4,  },
	{	0x145,	5,	4,  },
	{	0x151,	5,	4,  },
	{	0x154,	5,	4,  },
	//����
	{	0x54,	6,	5,  },
	{	0x114,	6,	5,  },
	{	0x144,	6,	5,  },
	{	0x150,	6,	5,  },
	//����
	{	0x15,	5,	6,  },
	{	0x150,	5,	6,  },
	{	0x51,	5,	6,  },
	{	0x114,	5,	6,  },
	{	0x45,	5,	6,  },
	{	0x144,	5,	6,  },
	//���
	{	0x140,	6,	7,  },
	{	0x50,	6,	7,  },
	{	0x14,	6,	7,  },
	{	0x110,	6,	7,  },
	{	0x44,	6,	7,  }
};

BYTE MatchTable[MATCH_SIZE];

static BYTE MatchTemplate( register int bitset, register int width )
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
					//8��ʾ�ܵ�ģ������ + 1
					return Compare[cx].offset + 8;
				}
			}
		}
	}
	{
	/*
	 *֮����forѭ��Ҫ��1��Ϊ�˱�������һ�ֻ�����ģ�嵼�µ��жϴ������£�
	 *���� 1 0 1 0 1 0 0 1 ������������ 0 1 0 1 0 1 ��ģ�����ƥ��
	 *�������һ����ôƥ��ʱ�������е� �Ƚ� 1 0 1 0 1 0 �� 0 1 0 1 0 1 ʱ��Ϊ����ȷ���ʧ��
	 */
	int fragfive[22], fragsix[20];

	for ( cx = 0; cx <= 2 * width - 10 + 1; cx ++ )
	{
		fragfive[cx] = bitset >> cx & FIVE_MASK;
	}
	for ( cx = 0; cx <= 2 * width - 12 + 1; cx ++ )
	{
		fragsix[cx] = bitset >> cx & SIX_MASK;
	}
	//ƥ��ɹ�ʱ��axΪ������˵����ҵ�ģ��ƥ��ɹ���Ϊż����˵���������ģ��ƥ��ɹ���ͨ��ʹ����ֵ����һ������8������
	for ( cx = 0; cx < TEMPLATE_CNT; cx ++ )
	{
		if ( Compare[cx].width == 5 )
		{
			for ( ax = 0; ax <= 2 * width - 10 + 1; ax ++ )
			{
				if ( Compare[cx].templ == fragfive[ax] )
				{
					return Compare[cx].offset + ( ax & 1 ? 8 : 0 );
				}
			}
		}
		else
		{
			for ( ax = 0; ax <= 2 * width - 12 + 1; ax ++ )
			{
				if ( Compare[cx].templ == fragsix[ax] )
				{
					return Compare[cx].offset + ( ax & 1 ? 8 : 0 );
				}
			}
		}
	}
	}
	return 0;
}
static int BitDecompre( int input , int width )
{
	register int	offset = 0;
	register int	output = 0;
	while( offset < width )
	{
		output |= (input % 3) << (2 * offset);
		input  /= 3; 
		offset ++;
	}
	return output;
}
static int LibraryMake( HANDLE hFile )
{
	int cx, ax;
	DWORD	dwWrite;
	memset( MatchTable, 0, MATCH_SIZE * sizeof(BYTE));

	//����������Ժϳ�һ��ѭ����������Ȼд���˾����˰ɣ�
	for ( cx = OFFSET_FIT; cx < OFFSET_FOT; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_FIT, 15 );
		MatchTable[cx] = MatchTemplate( ax, 15 );
	}
	for ( cx = OFFSET_FOT; cx < OFFSET_THT; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_FOT, 14 );
		MatchTable[cx] = MatchTemplate( ax, 14 );
	}
	for ( cx = OFFSET_THT; cx < OFFSET_TWE; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_THT, 13 );
		MatchTable[cx] = MatchTemplate( ax, 13 );
	}
	for ( cx = OFFSET_TWE; cx < OFFSET_ELE; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_TWE, 12 );
		MatchTable[cx] = MatchTemplate( ax, 12 );
	}
	for ( cx = OFFSET_ELE; cx < OFFSET_TEN; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_ELE, 11 );
		MatchTable[cx] = MatchTemplate( ax, 11 );
	}
	for ( cx = OFFSET_TEN; cx < OFFSET_NIN; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_TEN, 10 );
		MatchTable[cx] = MatchTemplate( ax, 10 );
	}
	for ( cx = OFFSET_NIN; cx < OFFSET_EIG; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_NIN, 9 );
		MatchTable[cx] = MatchTemplate( ax, 9 );
	}
	for ( cx = OFFSET_EIG; cx < OFFSET_SEV; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_EIG, 8 );
		MatchTable[cx] = MatchTemplate( ax, 8 );
	}
	for ( cx = OFFSET_SEV; cx < OFFSET_SIX; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_SEV, 7);
		MatchTable[cx] = MatchTemplate( ax, 7 );
	}
	for ( cx = OFFSET_SIX; cx < OFFSET_FIV; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_SIX, 6 );
		MatchTable[cx] = MatchTemplate( ax, 6 );
	}
	for ( cx = OFFSET_FIV; cx < MATCH_SIZE; cx ++ )
	{
		ax = BitDecompre( cx - OFFSET_FIV, 5 );
		MatchTable[cx] = MatchTemplate( ax, 5 );
	}
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	WriteFile( hFile, MatchTable, MATCH_SIZE * sizeof(BYTE), &dwWrite, NULL );
	return 0;
}

int main( )
{
	HANDLE hFile =  CreateFile( TEXT("Dictionary.DAT"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	LibraryMake( hFile );
	CloseHandle( hFile );
	return 0;
}