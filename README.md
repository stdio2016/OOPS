# OOPS
OOPS! 這個語言好難。但這才是純物件導向語言

OOPS 是 Object Orient Programming System 的簡稱，裡面只有跟物件導向相關的指令，以及必要的標準輸入輸出指令

這裡提供一個 OOPS 語言的直譯器，讓你實際使用 OOPS

## 版本紀錄
* 2018/7/17：直譯器寫出來了
* 2018/7/21：加入垃圾收集機制
* 2018/7/22：加入堆疊檢查

## 特色
OOPS 語言中只有一種資料型態，就是物件。OOPS 實現了物件導向的封裝、繼承、多型等特性

### 物件與類別
OOPS 屬於類別導向 (class-based)，所以在 OOPS 裡，要產生物件 (object)，你必須要先宣告類別 (class)

類別可以想像成是設計圖，物件則可以想像成是成品。類別敘述物件可以做的事情，按照物件導向設計的用語，這稱為方法 (method)

### 訊息傳遞
為了讓物件能夠真的做到想要的功能，可以要求物件執行方法，稱作呼叫 (call)。OOPS 的呼叫語法是 `obj.methodName(args)`，其中 obj 是指定的物件，methodName 是方法的名稱，args 是參數

在 OOPS 裡，所有的方法都是公有 (public) 的，這表示，任何的程式都能呼叫這些方法

### 封裝
在 OOPS 裡，每個物件有自己的變數，稱為成員變數 (member variable)，而且只有自己才能更改這些變數，因為它們都是私有 (*private*) 成員變數

### 繼承
OOPS 的類別 (class) 是可以繼承的。只要使用繼承，子類別 (subclass) 就可以使用父類別 (parent class) 的方法和成員變數。

如果需要，子類別也可以覆寫父類別的方法，因為所有的方法都是虛擬 (virtual) 的

### 多型
OOPS 支援多型，而且這大概是最常用到的物件導向特性，因為，啊～OOPS 沒有迴圈，也沒有判斷式語法 (我認為有了這些功能就不是純物件導向了)

## 編譯方法
請先安裝 bison 和 flex，一定要 bison ，因為我用了 bison 才有的功能 `%destructor`，還有一定要 flex，因為 lex 年代太久遠，有很多功能都不支援

進入 `src` 資料夾，然後在命令列下輸入 `make`

然後就會在資料夾裡面出現 `oops` 程式了！耶！

要在 Windows 底下編譯這個程式，需要先安裝 Cygwin，可以只安裝裡面的 flex、bison 和 gcc。
編譯輸入的指令要改成 `make CFLAGS="-L C:\cygwin64\lib"`，如果你安裝 Cygwin 的位置是 `C:\cygwin64`。
還有，要記得把 Cygwin 加進 PATH 環境變數

## 執行方法
用法： `./oops <source file>`

執行 OOPS 程式碼
