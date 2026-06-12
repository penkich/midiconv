# include <MIDI.h>
//#include <utility>

//MIDI_CREATE_DEFAULT_INSTANCE();

//const int MIDI_RX_PIN = 12; 
const int MIDI_RX_PIN = 41;
//const int MIDI_TX_PIN = 13;
const int MIDI_TX_PIN = 42;
//MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, Name, Settings)
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

uint8_t Part = 1;
uint8_t Notes[] = {0,0,0,0}; // 同時発音数＝４を確保するための配列（ゼロが空き部屋）
int8_t Point = 0; // 空き部屋の位置
uint8_t D1 = 0; // 周波数が入る

int8_t InpNotes(uint8_t value){
    if(SearchNotesZero() >=0){
        Point = SearchNotesZero();
        Notes[Point] = value;
        return 1;
    }
    return -1;
}

int8_t SearchNotesZero(){
    for(uint8_t i=0; i<4; i++){
        if(Notes[i] == 0){
            return i;
        }
    }
    return -1;
}

int8_t SearchNotes(uint8_t number){
    for(uint8_t i=0; i<4; i++){
        if(Notes[i] == number){
            return i;
        }
    }
    return -1;
}

float genFreq(uint8_t number){
    return 440.0 * pow(2.0, (number - 69) / 12.0);
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // ノートの鳴り始めにさせたいことを何でも書いて
    D1 = MIDI.getData1();
    if(InpNotes(D1) >=0){
        Serial.printf("0, %.3f,n%d\n", genFreq(D1), Point+1);
        Serial1.printf("0, %.3f,n%d\n", genFreq(D1), Point+1);
    }
    // できるだけコールバックの中は短くするように、
    // でないとloop関数が遅くなり、リアルタイム性能に悪影響が出てしまう
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // ノートの鳴り終わりにさせたいことを何でも
    D1 = MIDI.getData1();
    int8_t t = SearchNotes(D1);
    if(t >=0){
        Notes[t] = 0;
        Point = t;
        Serial.printf("0, 0,n%d\n", Point+1);
        Serial1.printf("0, 0,n%d\n", Point+1);
    }
    // ベロシティ0のノートオンメッセージもNoteOffとして解釈される(つまりここで処理される)点に注意
}

void setup()
{
    //Serial1.setPins(MIDI_RX_PIN, MIDI_TX_PIN);
    Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
    Serial.begin(115200);
    Serial.println("start");

//    Serial1.setTX(0);
//    Serial1.setRX(1);
//    Serial1.setCTS(2); // CTSのGPIO（ラズパイピコでは使えるポートに制限あり）
//    Serial1.setRTS(3); // RTSのGPIO（ラズパイピコでは使えるポートに制限あり）
    //uart_set_hw_flow(uart0, true, true); // RTS/CTSフロー制御
//    uart_set_hw_flow(uart0, false, false); // RTS/CTSフロー制御なし
    Serial.println("debug2");
//    Serial1.setFIFOSize(12800);  // バッファー
    //Serial1.begin(31250); // 音データ受信用 UART初期化
    //Serial1.begin(115200); // 音データ受信用 UART初期化
    // handleNoteOn関数をMIDIライブラリに結びつけ、NoteOn時に呼び出されるようにする
    MIDI.setHandleNoteOn(handleNoteOn);  // 引数にはハンドラの「関数名」を書く

    // NoteOffにも同様
    MIDI.setHandleNoteOff(handleNoteOff);

    // 全チャンネルを読む形でMIDIインスタンスを初期化
    MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop()
{
    // リアルタイム性の確保のため、MIDI.readをできるだけ早く呼ぶように
    MIDI.read();
}
