
#ifndef GUI_H
#define GUI_H




void GUI_DrawPoint(u16 x,u16 y,u16 color);
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void Draw_Circle(u16 x0,u16 y0,u16 fc,u8 r);
void Draw_Triangel(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2);
void Fill_Triangel(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2);
void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode);
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len);
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode);
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode);
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
 
void gui_circle(int xc, int yc,u16 c,int r, int fill);
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2);

void LCD_Show_2412_char(u16 x,u16 y,u16 fc, u16 bc, u8 num, u8 mode);
void LCD_ShowString2412(u16 x,u16 y,u8 *p,u8 mode);
void LCD_ShowNum2412(u16 x,u16 y,u32 num,u8 len);

void LCD_Show_3216_char(u16 x,u16 y,u16 fc, u16 bc, u8 num, u8 mode);
void LCD_ShowString3216(u16 x,u16 y,u8 *p,u8 mode);
void LCD_ShowNum3216(u16 x,u16 y,u32 num,u8 len);



void LCD_Show_4824_char(u16 x,u16 y,u16 fc, u16 bc, u8 num, u8 mode);
void LCD_ShowString4824(u16 x,u16 y,u8 *p,u8 mode);
void LCD_ShowNum4824(u16 x,u16 y,u32 num,u8 len);


void Show_Str4824(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 mode);
void Show_Str3216(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 mode);
void Show_Str2412(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 mode);


void Gui_Drawbmp(u16 x,u16 y,const unsigned char *p);
void Gui_Drawbmp16(u16 x,u16 y,const unsigned char *p);
void Gui_Drawbmp40(u16 x,u16 y,const unsigned char *p);
void Gui_Drawbmp48(u16 x,u16 y,const unsigned char *p);

void Gui_StrCenter3216(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 mode);


#endif

