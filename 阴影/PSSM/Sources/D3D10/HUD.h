#pragma once

// Draws other HUD stuff
//
void RenderHUD(void)
{
  // Build stats string
  //
  char strText[1024];
  PrintStats(strText);
  if(strText == 0) return;

  D3D10_VIEWPORT Viewport;
  unsigned int iNumVP = 1;
  GetApp()->GetDevice()->RSGetViewports(&iNumVP, &Viewport);
  RECT destRect;
  SetRect(&destRect, 10, 10, Viewport.Width - 10, 400);

  // draw stats
  //在win10下面，更新了d3d驱动之后，需要把这一句话屏蔽掉
  //否则会出现场景不完善bug
 // g_pFont->DrawTextA(NULL, strText, -1, &destRect, DT_NOCLIP, 0xFFFFFFFF);
}