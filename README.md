# X-font-maker
X　Window System 上で動くフォント作成・表示が可能なプログラム、ライブラリです。

## fontMakerの使い方
- fontMakerディレクトリ内で、以下のコマンドでコンパイルし，a.outを実行します。<br>
`cc fontMaker.c Flib/FFont.c Flib/FillMask.c -I /usr/include/ -L /usr/lib/ -l X11 -lm`
- スライムっぽいのが左上で動いてたら正常に動作しています。
- 各文字ごとにフォントを自分で描けます。
- それぞれのボタンは、押せば仕様がわかります。
- フォントの保存、読み込みができます。
- 日本語を含め、ほとんどの文字に対応しています。
- 縦に長いボタンで、フォントのデフォルトの文字間隔を変更できます。
- スライム君をクリックすると、幸せになれます。

## 作ったフォントの使い方
- デモを見るには、fontMakerディレクトリ内で、以下のコマンドでコンパイルし，a.outを実行します。<br>
`cc fontTest.c Flib/FFont.c Flib/FillMask.c -I /usr/include/ -L /usr/lib/ -l X11 -lm`
- 「foo ber」が表示されて、3つの画面が順番に表示されたら正常に動作しています。
- `Flib/FFont.h`と`Flib/FillMask.h`をインクルードして(fontTest.cのように)、FLoadFontで読み込むことで作ったフォントで描画できるようになります。

## 実行環境 (仮想マシン)
VirtualBox 6.1.32 r149290 (Qt5.6.2)
************************************
- Intel Core i5 3470 3.20GHz
- Memory 3.0GB
- gcc version 9.3.0
- OS Ubuntu 20.04.3 LTS
  
## 注意事項
- ソースファイルのコンパイルにはlibx11-devパッケージが必要です(ubuntu環境では`sudo apt-get install libx11-dev`でインストールできます)。
- このアプリケーションで作ったフォントはこのライブラリからしか読み込めないし、使えません。
