//#include "SD_Card.h"
#include <cstring>
#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "ui.h"
#include <EncButton.h>
#include "SD.h"

Button lbtn(0);
Button rbtn(1);

bool state = true; // true - menu, false - reading mode
unsigned int filesCnt = 0;
char myFilesArray[5][100] = {0};
unsigned int currentFileIndex = 0; // Индекс выбранного файла в меню
unsigned int currentDisplayedLine = 0; // Индекс строки, отображаемой в данный момент (для режима чтения)
File file;

// Переменные для кэширования текущей читаемой строки и ее фрагментов
String currentFullLine = "";
unsigned int currentLineFragmentIndex = 0; // Индекс текущего 40-символьного фрагмента внутри currentFullLine

const int MAX_LINE_LENGTH = 512; // Максимальная длина строки, которую мы ожидаем прочитать

void setup()
{       
  LCD_Init();
  Lvgl_Init();
  //SD_Init();
  pinMode(4, OUTPUT);    
  digitalWrite(4, HIGH);               
  if (SD.begin(4, SPI)) {
    printf("SD card initialization successful!\r\n");
  } else {
    printf("SD card initialization failed!\r\n");
  }
  SD.begin(4, SPI);   
  ui_init();
  // lv_demo_widgets();               
  // lv_demo_benchmark();          
  // lv_demo_keypad_encoder();     
  // lv_demo_music();  
  // lv_demo_stress();   

  //Wireless_Test2();  
  load_menu();
}

uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][100],uint16_t maxFiles) {
  File Path = SD.open(directory);
  if (!Path) {
    printf("Path: <%s> does not exist\r\n",directory);
    return false;
  }
  
  uint16_t fileCount = 0;
  char filePath[100];
  File file = Path.openNextFile();
  while (file && fileCount < maxFiles) {
    if (!file.isDirectory() && strstr(file.name(), fileExtension)) {
      strncpy(File_Name[fileCount], file.name(), sizeof(File_Name[fileCount])); 
      if (strcmp(directory, "/") == 0) {                                      
        snprintf(filePath, 100, "%s%s", directory, file.name());   
      } else {                                                            
        snprintf(filePath, 100, "%s/%s", directory, file.name());
      }
      printf("File found: %s\r\n", filePath);
      fileCount++;
    }
    file = Path.openNextFile();                                      
  }
  Path.close();                                                         
  if (fileCount > 0) {
    printf(" %d <%s> files were retrieved\r\n",fileCount,fileExtension);
    return fileCount;                                                 
  } else {
    printf("No files with extension '%s' found in directory: %s\r\n", fileExtension, directory);
    return 0;                                                         
  }
}

void load_menu(){
  lv_disp_load_scr(ui_Screen1);
  currentFileIndex = 0; // Сбрасываем индекс файла при загрузке меню
  filesCnt = 0;
  Folder_retrieval("/", ".txt", myFilesArray, 10);
  String allFiles = "";
  for (char (&file)[100]: myFilesArray){
    if (file[0] == '\0') { 
        break; 
    }
    filesCnt++;
    allFiles += file;
    allFiles += "\n";
  }
  lv_roller_set_options(ui_menu, allFiles.c_str() , LV_ROLLER_MODE_NORMAL);
}

void load_reader(char file_path[100]) {
  lv_disp_load_scr(ui_Screen2);
  currentDisplayedLine = 0; // Сбрасываем индекс отображаемой строки при загрузке нового файла
  currentLineFragmentIndex = 0; // Сбрасываем индекс фрагмента
  currentFullLine = ""; // Очищаем кэш строки

  String path = "/";
  path += file_path;
  file = SD.open(path, FILE_READ);
  if (!file) {
      lv_textarea_set_text(ui_text, "Failed to open file.");
      return;
  }
  update_display();
}

// Вспомогательная функция для получения текущей строки из файла
String getLineFromFile(File& f, unsigned int lineIndex) {
    if (!f) return "";

    f.seek(0); // Начинаем поиск с начала файла
    char buffer[MAX_LINE_LENGTH];
    unsigned int currentReadLines = 0;
    while (f.available() && currentReadLines <= lineIndex) {
        int bytesRead = f.readBytesUntil('\n', buffer, MAX_LINE_LENGTH - 1);
        buffer[bytesRead] = '\0'; // Добавляем нулевой символ для завершения строки
        if (currentReadLines == lineIndex) {
            String result = String(buffer);
            result.trim(); // Удаляем возможные пробелы или переводы строки в конце
            return result;
        }
        currentReadLines++;
        // Пропускаем символ новой строки, если он был
        if (f.peek() == '\n') {
            f.read();
        }
    }
    return ""; // Строка не найдена
}

void update_display(){
  if (state){ // Режим меню
    currentFileIndex = constrain(currentFileIndex, 0, filesCnt - 1);
    lv_roller_set_selected(ui_menu, currentFileIndex, LV_ANIM_OFF);
  } else { // Режим чтения
    if (!file || !file.available()) {
        lv_textarea_set_text(ui_text, "No file loaded or end of file.");
        return;
    }

    // Если мы перешли на новую "логическую" строку, или кэш пуст,
    // или индекс фрагмента стал отрицательным (переход на предыдущую строку)
    if (currentFullLine.length() == 0 || (currentLineFragmentIndex * 400 >= currentFullLine.length() && currentLineFragmentIndex != 0) || currentLineFragmentIndex < 0) {
        // Если индекс фрагмента стал отрицательным, значит, мы хотим перейти на предыдущую полную строку
        if (currentLineFragmentIndex < 0) {
            if (currentDisplayedLine > 0) {
                currentDisplayedLine--;
                currentFullLine = getLineFromFile(file, currentDisplayedLine);
                // Устанавливаем currentLineFragmentIndex на последний фрагмент предыдущей строки
                currentLineFragmentIndex = (currentFullLine.length() + 399) / 400 - 1; 
                if (currentLineFragmentIndex < 0) currentLineFragmentIndex = 0; // Гарантируем, что не будет отрицательным
            } else {
                currentLineFragmentIndex = 0; // Если это первая строка, остаемся на первом фрагменте
                currentFullLine = getLineFromFile(file, currentDisplayedLine); // Обновляем строку
            }
        } else { // Либо новая строка, либо закончились фрагменты текущей
            currentDisplayedLine++; // Переходим на следующую логическую строку в файле
            currentFullLine = getLineFromFile(file, currentDisplayedLine);
            currentLineFragmentIndex = 0; // Начинаем с первого фрагмента новой строки
        }
        
        // Если строка пуста после чтения, это может быть конец файла или ошибка.
        // Пробуем прочитать следующую, пока не найдем непустую или не достигнем конца файла
        while (currentFullLine.length() == 0 && file.available()) {
            currentDisplayedLine++;
            currentFullLine = getLineFromFile(file, currentDisplayedLine);
        }

        if (currentFullLine.length() == 0 && !file.available()) {
             lv_textarea_set_text(ui_text, "End of file.");
             return;
        }
    }
    
    // Определяем начало и конец фрагмента для отображения
    int start = currentLineFragmentIndex * 400;
    int end = start + 400;
    if (end > currentFullLine.length()) {
        end = currentFullLine.length();
    }
    
    String fragment = currentFullLine.substring(start, end);
    lv_textarea_set_text(ui_text, fragment.c_str());
    lv_obj_scroll_to_y(ui_text, 0, LV_ANIM_OFF); // Сброс прокрутки LVGL, чтобы всегда видеть начало фрагмента
  }
}

void loop()
{
  lbtn.tick();
  rbtn.tick();
  Timer_Loop(); // Убедитесь, что эта функция вызывает lv_timer_handler()

  if (lbtn.hold(0)){ // Долгое нажатие левой кнопки - возврат в меню
    state = true;
    load_menu();
  }
  if (rbtn.hold(0)) { // Долгое нажатие правой кнопки - переход в режим чтения
    state = false;
    load_reader(myFilesArray[currentFileIndex]); // Используем currentFileIndex для выбора файла
  }
  if (lbtn.click()){ // Короткое нажатие левой кнопки
    if (state) { // В режиме меню
      currentFileIndex--;
    } else { // В режиме чтения
      currentLineFragmentIndex--;
    }
    update_display();
  }
  if (rbtn.click()){ // Короткое нажатие правой кнопки
    if (state) { // В режиме меню
      currentFileIndex++;
    } else { // В режиме чтения
      currentLineFragmentIndex++;
    }
    update_display();
  }
}