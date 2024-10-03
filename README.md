# AdvancedCG
筑波大学情報学群情報メディア創成学類・情報科学類の授業「アドバンストCG」の第1回～第4回のレポートのためのベースコード公開用リポジトリです。

# 各フォルダについて
* Advanced01～Advanced04: 第1回～第4回の授業課題のベースプログラムが置いてあります。Visual Studio 2022 用のソリューションファイル (.sln) が入っていますので、Visual Studio 2022 で開発する場合はそちらを開いてください。
* Resources: 実行時に参照する画像やモデルファイルなどが置いてあります。
* include: 第1回～第4回の授業課題で必要となるヘッダファイルが置いてあります。Visual Studio 以外でコンパイルする際はこのディレクトリ (include) をインクルードフォルダとして追加するか、include 内の *.h ファイルをフォルダ構造そのままにcpp ファイルのあるところにコピーするようにしてください。
* vc-x64-lib-common: Visual Studio 共通のライブラリファイル
* vc-x64-libs: バージョン別のライブラリファイル

## 注意事項
### Windows
* もし自分の PC に Visual Studio 2022 がインストールされていない場合は，藤澤先生の講義資料フォルダ(SharePoint)の「[Visual Studio Community インストール方法.pdf](https://o365tsukuba.sharepoint.com/sites/msteams_087e13/Shared%20Documents/Forms/AllItems.aspx?id=%2Fsites%2Fmsteams%5F087e13%2FShared%20Documents%2FGeneral%2Fadvancedcg&p=true&ga=1 "Visual Studio Community インストール方法.pdf")」 を参考にしてインストールしてください．

### Mac, Linux (Ubuntu)
* 上記ソースコード一式は Mac 用 Makefile と Ubuntu 用 Makefile を src ディレクトリに同梱してありますのでそれを使ってください (Mac 用は Makefile_Mac としてあるので適宜ファイル名変更してください)。
