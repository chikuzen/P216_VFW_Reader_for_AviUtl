P21X VFW Reader for AviUtl version 0.2.0

Copyright (c) 2012 Oka Motofumi (chikuzen.mo at gmail dot com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


Q: これは何ですか？
A: AviUtl0.99l以降用のP216専用入力プラグインです。
   .vpy(vapoursynth script file)及び.aviをVideo for Windowsを使って読み込みます。


Q: P21Xってなんですか？
A: P210/P216のことをさします。
   P210/P216はMicrosoftが決めたYUV422-16bit用のFourCCです。
   P210はYUV各プレーンが10bitのYUVをビットシフトして16bitにしたもの、P216はそのまま16bitです。
   色の並び方はNV12とだいたい同じです。
   詳しくは
   http://msdn.microsoft.com/en-us/library/windows/desktop/bb970578(v=vs.85).aspx
   を読んで下さい。


Q: 具体的には？
A: .vpyか.aviを渡された時、fccHandlerが"P210"か"P216"であればこのプラグインで読み込めます。
   P210かP216以外のfccHandlerであれば何もしません。
   よって.vpyの場合は事前にlastクリップをYUV422P10かYUV422P16に変換しておく必要があります。
   P21XはYC48に変換してからAviUtlに渡されます。

   16bitYUV->YC48の変換式は次のようになっています。
      YC48_Y  = ((16bit_y - 4096) * 4789) / 256
      YC48_Cb = ((16bit_u - 32768) * 4683) / 256
      YC48_Cr = ((16bit_v - 32768) * 4683) / 256
   いずれも符号付き32bit整数で計算した後、符号付き16bit整数にキャストしています。
   なお、色差の水平方向は線形補間されます。


Q: 音声が読めません
A: VapourSynthが音声非対応なので、こちらも非対応です。


Q: その他注意する点は？
A: Video for Windowsを使っているので、2GBよりも大きなaviを読ませると途中で映像が切れます。
   (試したことないけど、理屈ではそうなります)



ソースコード:
https://github.com/chikuzen/P216_VFW_Reader_for_AviUtl/
