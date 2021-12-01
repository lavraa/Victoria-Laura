#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>





// Pines
const int pinXJ = A0;
const int pinYJ = A1;
const int DIN = 10;
const int CS = 12;
const int CLK = 11;
const int button = 9;
const int buzzer = 8;

// Variables, Estructuras y Objetos

// Objetos controladores de la matriz

MD_MAX72XX matriz(MD_MAX72XX::FC16_HW, DIN, CLK, CS, 4);
MD_Parola mp(MD_MAX72XX::FC16_HW, DIN, CLK, CS, 4);

// Estructuras

struct Punto{
  int fila = 0, col = 0;
  Punto(int fila = 0, int col = 0): fila(fila), col(col) {}
};

struct Coordinar{
  int x = 0, y = 0;
  Coordinar(int x = 0, int y = 0): x(x), y(y) {}
};

// Variables
Punto snake; // Coordenadas de la serpiente
Punto comida(-1, -1); // Coordenadas de la comida
Coordinar joystickPas(500, 500); //Valores default del Joystick
bool ganaste = false; // Variable que comprueba si gano
bool gameOver = false; // Variable que comprueba si perdió
const int velMensaje = 3000; // Velocidad a la que se desplazan los mensajes
const int snakeLongInicial = 3; // Tamaño inicial de la serpiente
int snakeLong = snakeLongInicial; // Longitud de la serpiente (cambia)
int snakeVelocipy = 300; // Velocidad de la serpiente
int snakeDir = 0; // Dirección de la serpiente
int puntaje; // Puntos que se ganan
const int arriba = 1; // Direccion hacia arriba
const int derecha = 2; // Direccion hacia derecha
const int abajo = 3; // Direccion hacia abajo
const int izquierda = 4; // Direccion hacia izquierda
const int joystickOK = 160; // A partir de que valor se toma en cuenta el movimiento del Joystick
int tablero[8][32] = {}; // Guarda el tablero


//Funciones

void setup() {
  /*Setup que llama a las variables de inicio*/
  start();
  calibrarJoystick();
  mensajeSnake();
}

void loop() {
  /*Función que se repite infinitamente*/
  spawnMuros();
  generarComida();
  setJoystick();
  movSnake();
  gameState();
}

void start(){
  /*Función que inicializa a las matrices y la posición del snake*/
  matriz.begin();
  mp.begin();
  
  snake.fila = 4;
  snake.col = 16;
}

void generarComida(){
  /*Función que que genera la comida y comprueba si se gana*/
  if(comida.fila == -1 || comida.col == -1){
    if(snakeLong >= 255){
      ganaste = true;
      return; // Previene la generación de la comida
    }

    do{
      comida.fila = random(8);
      comida.col = random(32);
      if(comida.fila == 3 || comida.fila == 4){
        if(comida.col == 3 || comida.col == 4 || comida.col == 11 || comida.col == 12 || comida.col == 20 || comida.col == 21 || comida.col == 27 || comida.col == 28){
          comida.col += 2;
          comida.fila += 2;
        }
      }
    }while(tablero[comida.fila][comida.col] > 0);
  }
}

void setJoystick(){
  /*Función que establece el funcionamiento del Joystick*/
  int dirPrevia = snakeDir; // Variable que guarda la dirección anterior
  long tiempo = millis(); // El tiempo se va a calcular en milisegundos
  int X = analogRead(pinXJ);
  int Y = analogRead(pinYJ);

  while(millis() < tiempo + snakeVelocipy){
    
    if(snakeVelocipy == 0){
      // Este if se asegura que la velocidad nunca llegue a cero, para que no ocurran errores
      snakeVelocipy = 1;
    }

    Y < joystickPas.y - joystickOK ? snakeDir = arriba    : 0;
    Y > joystickPas.y + joystickOK ? snakeDir = abajo  : 0;
    X < joystickPas.x - joystickOK ? snakeDir = izquierda  : 0;
    X > joystickPas.x + joystickOK ? snakeDir = derecha : 0;

    // ignore directional change by 180 degrees (no effect for non-moving snake)
    snakeDir + 2 == dirPrevia && dirPrevia != 0 ? snakeDir = dirPrevia : 0;
    snakeDir - 2 == dirPrevia && dirPrevia != 0 ? snakeDir = dirPrevia : 0;
    
    

    // Hace que la comida parpadee para poder diferencialarla de la serpiente y del muro
    matriz.setPoint(comida.fila, comida.col, millis() % 100 < 50 ? 1 : 0);
  }
}

void movSnake(){
  /*Se encarga de direccionar a la serpiente y que la misma se mueva, así como de sus game over*/
  switch(snakeDir){
    case arriba:
      snake.fila--;
      limitesManager();
      matriz.setPoint(snake.fila, snake.col, 1);
      break;

    case derecha:
      snake.col--;
      limitesManager();
      matriz.setPoint(snake.fila, snake.col, 1);
      break;

    case abajo:
      snake.fila++;
      limitesManager();
      matriz.setPoint(snake.fila, snake.col, 1);
      break;

    case izquierda:
      snake.col++;
      limitesManager();
      matriz.setPoint(snake.fila, snake.col, 1);
      break;

    default:
      return;
  }

  if(tablero[snake.fila][snake.col] > 1 && snakeDir != 0){
    // Si se choca contra su cola, muere
    gameOver = true;
    return;
  }

  if(snake.fila == 3 || snake.fila == 4){
    if(snake.col == 3 || snake.col == 4 || snake.col == 11 || snake.col == 12 || snake.col == 20 || snake.col == 21 || snake.col == 27 || snake.col == 28){
      // Si colisiona con un muro, también muere
      gameOver = true;
      return;
    }
  }
  

  if(snake.fila == comida.fila && snake.col == comida.col){
    // Comprueba que la serpiente haya comido
    snakeVelocipy -= 2; // Aumenta la velocidad disminuyendo el tiempo en que se mueve
    comida.fila = -1;  // Resetea la comida
    comida.col = -1;
    
    snakeLong++; // Crece la serpiente

    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 32; col++) {
        if (tablero[fila][col] > 0 ) {
          tablero[fila][col]++;
        }
      }
    }
  }

  tablero[snake.fila][snake.col] = snakeLong + 1;

  for (int fila = 0; fila < 8; fila++) {
    for (int col = 0; col < 32; col++) {
      if (tablero[fila][col] > 0 ) {
        tablero[fila][col]--;
      }

      // Enciende el nuevo led de la serpiente
      matriz.setPoint(fila, col, tablero[fila][col] == 0 ? 0 : 1);
    }
  }
}

void limitesManager(){
  /*Función que controla que la serpiente entre por un límite y salga por el contrario*/
  snake.col < 0 ? snake.col += 32 : 0;
  snake.col > 31 ? snake.col -= 32 : 0;
  snake.fila < 0 ? snake.fila += 8 : 0;
  snake.fila > 7 ? snake.fila -= 8 : 0;
}

void gameState(){
  /*Comprueba en que estado se encuentra el juego*/
  if(gameOver == true || ganaste == true){
    byeSnake();

    puntaje = snakeLong - snakeLongInicial;
    
    mostrarScore(puntaje);

    if(gameOver == true){
      mostarGameOver();
    }else if(ganaste == true){
      mostrarGanaste();
    }

    resetVariables();
  }
}

void byeSnake() {
  /*Animación que se muestra cuando se termina el juego*/
  // Apaga los leds
  matriz.setPoint(comida.fila, comida.col, 0);

  delay(800);

  // Hace un flash en la pantalla 5 veces
  for (int i = 0; i < 5; i++) {
    // Invierte la pantalla
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 32; col++) {
        matriz.setPoint(fila, col, tablero[fila][col] == 0 ? 1 : 0);
      }
    }

    delay(20);

    // La devuelve a su estado original
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 32; col++) {
        matriz.setPoint(fila, col, tablero[fila][col] == 0 ? 0 : 1);
      }
    }
    delay(50);
  }
  
  delay(600);

  for (int i = 1; i <= snakeLong; i++) {
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 32; col++) {
        if (tablero[fila][col] == i) {
          matriz.setPoint(fila, col, 0);
          delay(100);
        }
      }
    }
  }
}

void resetVariables(){
  /*Función que resetea las variables para un nuevo juego*/
  ganaste = false;
  gameOver = false;
  snake.fila = 4;
  snake.col = 16;
  comida.fila = -1;
  comida.col = -1;
  snakeLong = snakeLongInicial;
  snakeDir = 0;
  memset(tablero, 0, sizeof(tablero[0][0]) * 8 * 32);
  matriz.clear(0, 3);
  spawnMuros();
}

void calibrarJoystick(){
  /*Calibra al Joystick*/
  Coordinar var;

  for(int i = 0; i < 10; i++){
    var.x += analogRead(pinXJ);
    var.y += analogRead(pinYJ);
  } 

  joystickPas.x = var.x / 10;
  joystickPas.y = var.y / 10;
}

void spawnMuros(){
  /*Funcion que genera los muros contra los que se choca la serpiente*/
  matriz.setPoint(3, 3, true);
  matriz.setPoint(3, 4, true);
  matriz.setPoint(4, 3, true);
  matriz.setPoint(4, 4, true);

  matriz.setPoint(3, 11, true);
  matriz.setPoint(3, 12, true);
  matriz.setPoint(4, 11, true);
  matriz.setPoint(4, 12, true);

  matriz.setPoint(3, 20, true);
  matriz.setPoint(3, 21, true);
  matriz.setPoint(4, 20, true);
  matriz.setPoint(4, 21, true);

  matriz.setPoint(3, 27, true);
  matriz.setPoint(3, 28, true);
  matriz.setPoint(4, 27, true);
  matriz.setPoint(4, 28, true);
}

void mensajeSnake(){
  /*Función que muestra un mensaje al inicio*/
  mp.setTextAlignment(PA_CENTER);
  mp.print("SNAKE");
  delay(velMensaje);
  mp.displayClear(); 
}

void mostarGameOver(){
  /*Función que muestra un mensaje cuando perdemos*/
  mp.setTextAlignment(PA_CENTER);
  mp.print("U LOSE");
  delay(velMensaje);
  mp.displayClear();
}

void mostrarGanaste(){
  /*Función que muestra un mensaje cuando ganamos*/
  mp.setTextAlignment(PA_CENTER);
  mp.print("U WIN");
  delay(velMensaje);
  mp.displayClear();
}

void mostrarScore(int score){
  /*Función que muestra nuestro puntaje. Su parámetro recibe el puntaje*/
  mp.setTextAlignment(PA_CENTER);
  mp.print("SCORE:");
  delay(velMensaje);
  mp.print(score);
  delay(velMensaje);
  mp.displayClear();
}

void musicaGanar(){





  
}




