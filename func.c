char sRow=8,sCol=132;
//разрешение экрана (высота 1 байт = 8 пикселей, тоесть если дисплей 64 пикслея, задаем 8), по горизонтали оставляем как есть

unsigned char Buffer[sRow][sCol];  //unsigned char Buffer[sRow][sCol]; 



//---------------------------------------------------------------------
// Вывод картинки 128х64 на экран 132х64
// 2 пиксела слева и справа остаются незакрашенными
//--------------------------------------------------------------------------
void glcdDraw(const char* textPtr, uint8_t color) {   // для RDX154
    uint8_t  i, j;

    for(i=0; i<8; ++i) {
        for(j=0; j<132; ++j) {
            if((j>=2)&&(j<=129))
                if(color) Buffer[i][j] = *textPtr++;
                else      Buffer[i][j] = 0xff-*textPtr++;
            else          Buffer[i][j] = 0x00;
//              if(color) Buffer[i][j] = 0xff; // по 2 пикселя слева и справа
//              else      Buffer[i][j] = 0x00;
        }                                    // закрашиваем черным
    }
    glcdUpdate();
}

//--------------------------------------------------------------------------
// Назначение: Включить/выключить пиксель   on/off
// Входные данные: 1) х,y - координата пикселя  3) цвет - on/off
//--------------------------------------------------------------------------
void glcdPixel (unsigned char x, unsigned char y, unsigned char color){
    if (color) Buffer[y/8][x] |=   (1<<(y%8));
        else   Buffer[y/8][x] &= (~(1<<(y%8)));
}
//--------------------------------------------------------------------------
void glcdClear(uint8_t color) {
    uint8_t  i, j;

    for(i=0; i<8; ++i) {
        for(j=0; j<sCol/*128*/; ++j) {
            if(color) Buffer[i][j] = 0xff;
                else  Buffer[i][j] = 0x00;
        }
    }
}
//--------------------------------------------------------------------------
// Purpose:       Draw a line on a graphic LCD using Bresenham's
//                line drawing algorithm
// Inputs:        (x1, y1) - the start coordinate
//                (x2, y2) - the end   coordinate
//                color - ON or OFF
// Dependencies:  glcdPixel()
//--------------------------------------------------------------------------
//  Материал из Викиучебника — открытых книг для открытого мира
void glcdLine (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) {
    int dx=abs(x2-x1);    int signX=x1<x2 ? 1 : -1;
    int dy=abs(y2-y1);    int signY=y1<y2 ? 1 : -1;
    int error = dx - dy;

    glcdPixel(x2, y2, color );
    while(x1 != x2 || y1 != y2) {
        glcdPixel(x1, y1, color );
        int error2 = error * 2;
        if(error2>-dy) {error -= dy;   x1 += signX; }
        if(error2< dx) {error += dx;   y1 += signY; }
    }
}
//--------------------------------------------------------------------------
//  алгоритм Брезенхэма для рисования окружностей
//  x0, y0 - координаты центра,   radius - радиус,
//  fill(Заполнение) - on/off,    color - on или off
void glcdCircle(uint16_t x0, uint16_t y0, uint16_t radius,
                             uint8_t fill,uint8_t color)   {
    int x=0;   int y=radius;   int error=0;   int delta=1-2*radius;
    while (y >= 0) {
        if(fill) {
            glcdLine (x0+x, y0+y, x0+x, y0-y,color);
            glcdLine (x0-x, y0+y, x0-x, y0-y,color);
        } else {
	       glcdPixel (x0+x, y0+y, color);
	       glcdPixel (x0+x, y0-y, color);
	       glcdPixel (x0-x, y0+y, color);
	       glcdPixel (x0-x, y0-y, color);
        }
       error = 2*(delta+y)-1;
       if ((delta<0) && (error<=0))  {delta += 2 * ++x + 1; continue;}
       error = 2*(delta-x)-1;
       if ((delta>0) && (error> 0))  {delta += 1 - 2 * --y; continue;}
       x++;
       delta += 2*(x-y);
       y--;
   }
}
//--------------------------------------------------------------------------
// Назначение:      рисует прямоугольник, (x1,y1),(X2,y2) - координати
// fill  - заполнение - on/off,  color - Цвет - on/off
// Используемые процедуры: glcdPixel (), glcdLine ()
//--------------------------------------------------------------------------
void glcdRectangle(uint16_t x1,uint16_t y1, uint16_t x2,
                   uint16_t y2,uint8_t fill,uint8_t color) {
    if(fill) {
        uint16_t   i, yy1, yy2;
        if(y1<y2) {yy1=y1;  yy2=y2;}  else {yy1=y2;  yy2=y1;}
        for(i=yy1; i <= yy2; i++)
            glcdLine(x1, i, x2, i, color);
    } else {
        glcdLine(x1, y1, x2, y1, color);
        glcdLine(x1, y2, x2, y2, color);
        glcdLine(x1, y1, x1, y2, color);
        glcdLine(x2, y1, x2, y2, color);
    }
}
//==========================================================================
// Назначение:      Вывести текст на графический ЖК-дисплей
// Входные данные:  (х,у) - координата (левая верхняя) первой буквы
//                  textPtr - указатель на массив текста для отображения
//                  size - размер текста: 1 = 5x7, 2 = 10x14, ...
//                  color(Цвет) - on/off
//--------------------------------------------------------------------------
void glcdText(uint16_t x,uint16_t y, const char* textPtr,uint8_t size, uint8_t color){
    uint8_t  i, j, k, m;    uint8_t	 nSimv;

    for(; *textPtr; ++textPtr, ++x) {
        nSimv=*textPtr;
        if(nSimv=='\n') y += 7*size+1;          // Переход на новую строку
        if(nSimv=='\r') x = 0;                  // Возврат каретки
        if(x+5*size >= sCol) {x=0; y +=7*size+1;} //достигнута граница экрана

        nSimv -= (nSimv<0x80) ? 0x20 : 0x60;      // "Истинный" код символа
        for(i=0; i<5; ++i, x+=size) {
            for(j=0; j < 7; ++j) {
                if(symbol[nSimv][i] & (1<<j)) {
                    for(k=0; k < size; ++k) {
                        for(m=0; m < size; ++m) {
                            glcdPixel (x+m, y+j*size+k, color);
                        }
                    }
                }
            }
        }
    }
}

void glcdUpdate () {
    int i, j;

    {
    	  I2C_TransferSeq_TypeDef    seq;
    	  I2C_TransferReturn_TypeDef ret;
    	  uint8_t                    i2c_read_data[2];
    	  uint8_t                    i2c_write_data[3];

    	  seq.addr  = 0x70;
    	  seq.flags = I2C_FLAG_WRITE;
    	  /* Select command to issue */
    	  i2c_write_data[0]= 0xB0;
    	  i2c_write_data[1] = 0x00;
    	  i2c_write_data[2] = 0x10;

    	  seq.buf[0].data   = i2c_write_data;
    	  seq.buf[0].len    = 3;
    	  /* Select location/length of data to be read */

    	  ret = I2CSPM_Transfer(I2C0, &seq);

    	  //return((int) 2);

    }

	//i2c1Start(0x70,1,0);      // 0x70 адрес индикатора, данные, запись
	for (i=0; i<8; i++){                       //   8
		for (j=0; j<132/*sCol*/; j++)  {          // 132
			hmc5983_Write_Command_1(I2C0,0x72,Buffer[i][j]); //i2c1Write(Buffer[i][j]);
		}
	}

}
