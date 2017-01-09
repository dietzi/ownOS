void displaycursor(int row,int col)
{
  unsigned short tmp=row*80+col;
  outb(0x3D4,14);
  outb(0x3D5,tmp >> 8);
  outb(0x3D4,15);
  outb(0x3D5,tmp);
}