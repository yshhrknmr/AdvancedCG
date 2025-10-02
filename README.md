# Advanced CG / アドバンストCG
筑波大学情報学群情報メディア創成学類・情報科学類の授業「アドバンストCG」の第1回～第4回のレポートのためのベースコード公開用リポジトリです。第5回～第8回については[藤澤先生のリポジトリ](https://github.com/fujis/advancedcg)にアクセスしてください。

## 各フォルダについて
* Advanced01～Advanced04: 第1回～第4回の授業課題のベースプログラムが置いてあります。Visual Studio 2022 用のソリューションファイル (.sln) が入っていますので、Visual Studio 2022 で開発する場合はそちらを開いてください。
* Resources: 実行時に参照する画像やモデルファイルなどが置いてあります。
* include: 第1回～第4回の授業課題で必要となるヘッダファイルが置いてあります。Visual Studio 以外でコンパイルする際はこのディレクトリ (include) をインクルードフォルダとして追加するか、include 内の *.h ファイルをフォルダ構造そのままにcpp ファイルのあるところにコピーするようにしてください。
* vc-x64-lib-common: Visual Studio 共通のライブラリファイル
* vc-x64-libs: Visual Studio のバージョン別のライブラリファイル

## 自分のパソコンで環境構築する方法
### Windows
* もし自分の PC に Visual Studio 2022 がインストールされていない場合は，藤澤先生の講義資料フォルダ(SharePoint)の「[Visual Studio Community インストール方法.pdf](https://o365tsukuba.sharepoint.com/sites/msteams_087e13/Shared%20Documents/Forms/AllItems.aspx?id=%2Fsites%2Fmsteams%5F087e13%2FShared%20Documents%2FGeneral%2Fadvancedcg&p=true&ga=1)」 (アクセスできない場合は [こちら](https://drive.google.com/file/d/1ttns73NxB1s7RcrTviIYoe13l_8cBwl7/view?usp=sharing)) を参考にしてインストールしてください。

### Linux (Ubuntu)
1. Ubuntu のパッケージマネージャ apt で必要なパッケージをインストールします。「パッケージが見つからない」というエラーが出る場合は、
```
$ sudo apt-get update --with-missing
```
を行う必要があるかもしれません。必要なパッケージのインストールは次のコマンドでできます。
```
$ sudo apt install libdevil-dev libglfw3-dev libglew-dev
```
2. GitHub レポジトリからダウンロードしたファイル一式のうち、各回のフォルダ (Advanced0*) の src フォルダに移動してください。
```
$ cd Advanced01/src   # 1 回目の場合
```
3. Makefile をコピーしたディレクトリ内に移動し、次のコマンドを実行してください。
```
$ make run
```
これで課題のプログラムがコンパイルされて起動するはずです。

### Mac
1. 開発環境である Xcode をインストールします (※Visual Studio for Mac や Visual Studio Code などでも開発できると思いますが、以下の説明は Xcode を前提とします)。Xcode は App Store からダウンロードできます。
2. Terminal.app を起動し、パッケージマネージャ [Homebrew](https://brew.sh/) をインストールしてください。
```
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
3. Homebrew で必要なパッケージをインストールします。
```
$ brew install devil glfw glew
```
4. パッケージをインストールしただけでは各種パスが正しく設定されていないので、設定を行います。ホームディレクトリ直下の .zshrc ファイルに以下を記述してください。
```
export CPATH=/opt/homebrew/include    # M1 より古い Mac を使っている場合は CPATH=/opt/local/include
export LIBRARY_PATH=/opt/homebrew/lib      # M1 より古い Mac を使っている場合は LIBRARY_PATH=/opt/local/lib
```
記述してファイルを保存したら設定を読み込みます。
```
$ source ~/.zshrc
```
5. GitHub レポジトリからダウンロードしたファイル一式のうち、各回のフォルダの src フォルダに移動し、同梱の Makefile_Mac を Makefile とリネームしてください。
```
$ cd Advanced01/src    # 第 1 回の場合
$ mv Makefile Makefile_Ubuntu    # すでに Ubuntu 用の Makefile があるのでリネーム
$ mv Makefile_Mac Makefile
```
なお、リネームが面倒臭ければ `make` コマンド時に `-f Makefile_Mac` とファイル名を指定することもできます。

6. Terminal.app で次のコマンドを実行してください。
```
$ make run
```
これで課題のプログラムがコンパイルされて起動するはずです。なお、Mac 版の GLSL のコードは GLSL_Mac というフォルダに入っています。「[Mac でのその他の注意事項](https://github.com/yshhrknmr/AdvancedCG/tree/main#mac-%E3%81%A7%E3%81%AE%E3%81%9D%E3%81%AE%E4%BB%96%E3%81%AE%E6%B3%A8%E6%84%8F%E4%BA%8B%E9%A0%85)」も参照してください。

> [!CAUTION]
> 第3回では並列処理を行うための OpenMP ライブラリが必要です。

#### 第 3 回の課題で Mac で OpenMP を使うための設定
上記の Mac の設定に加えて、OpenMP のパッケージをインストールします。
```
$ brew install libomp
$ brew link libomp --force    # シンボリックリンクの作成
```
あとは上記の手順 5 以降を行ってください。もし M1 CPU よりも古い Mac を使っている場合は、手順 5 の後で `Makefile` 内の `-L/opt/homebrew/opt/libomp/lib` を `-L/opt/local/lib/libomp` に書き換えてください。

#### Mac でのその他の注意事項
* Mac 版の GLSL のコードには #version 130 ではなく #version 150 core を使用しており、テクスチャ画像を参照するための関数は texture2D() ではなく texture() を使ってください。これ以外にもバージョンを変えたことにより使うべき関数を変える必要が出てくるかもしれません。
