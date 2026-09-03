/*
  Automated 8-bit TTL adder/subtractor demonstration

  Serial Monitor commands:
  START = start the five tests
  STOP  = stop the test sequence

  Connections:
  Pins 3–10 = shared 8-bit bus
  Pin 3     = LSB
  Pin 10    = MSB
  Pin 11    = 74LS75 latch enable
  Pin 12    = ADD/SUB control

  Pin 12 LOW  = addition
  Pin 12 HIGH = subtraction

  Physical output:
  COUT + S7 through S0 = 9 LEDs
*/

const byte dataPins[8] = {
  3, 4, 5, 6, 7, 8, 9, 10
};

const byte latchPin = 11;
const byte subtractPin = 12;

// First operand for each test
const byte A[5] = {
  5, 15, 255, 8, 3
};

// Second operand for each test
const byte B[5] = {
  3, 1, 1, 3, 5
};

/*
  false = addition
  true  = subtraction
*/
const bool subtractMode[5] = {
  false,
  false,
  false,
  true,
  true
};

bool running = false;


// Send an 8-bit number to pins 3–10.
void writeBus(byte value) {
  for (byte bit = 0; bit < 8; bit++) {
    digitalWrite(
      dataPins[bit],
      bitRead(value, bit)
    );
  }
}


// Print a number as eight binary digits.
void printBinary8(byte value) {
  for (int bit = 7; bit >= 0; bit--) {
    Serial.print(bitRead(value, bit));
  }
}


// Load and latch operand A.
void latchA(byte value) {
  // Disable subtraction while loading A.
  digitalWrite(subtractPin, LOW);

  // Keep the latch closed while setting the bus.
  digitalWrite(latchPin, LOW);
  writeBus(value);

  delayMicroseconds(20);

  // 74LS75 is transparent while enable is HIGH.
  digitalWrite(latchPin, HIGH);

  delayMicroseconds(20);

  // Going LOW stores operand A.
  digitalWrite(latchPin, LOW);
}


// Perform and display one test.
void runTest(byte test) {
  byte expectedResult;
  bool expectedCarry;

  // Store A in the 74LS75 latch.
  latchA(A[test]);

  if (subtractMode[test]) {
    /*
      Select subtraction.

      The TTL circuit calculates:
      A + inverted B + 1
    */
    digitalWrite(subtractPin, HIGH);
    writeBus(B[test]);

    expectedResult = A[test] - B[test];

    /*
      During unsigned subtraction:

      COUT = 1 means no borrow
      COUT = 0 means borrow occurred
    */
    expectedCarry = A[test] >= B[test];
  }
  else {
    // Select addition.
    digitalWrite(subtractPin, LOW);
    writeBus(B[test]);

    /*
      Use 16 bits so the ninth carry bit
      is not discarded.
    */
    uint16_t fullResult =
      (uint16_t)A[test] + B[test];

    expectedResult = fullResult & 0xFF;
    expectedCarry = bitRead(fullResult, 8);
  }

  // Allow the TTL outputs to settle.
  delayMicroseconds(100);

  Serial.println();
  Serial.println("----------------------------");

  Serial.print("TEST ");
  Serial.print(test + 1);
  Serial.println(" OF 5");

  Serial.print("MODE:          ");

  if (subtractMode[test]) {
    Serial.println("SUBTRACTION");
  }
  else {
    Serial.println("ADDITION");
  }

  Serial.print("A:             ");
  printBinary8(A[test]);
  Serial.print("  (");
  Serial.print(A[test]);
  Serial.println(")");

  Serial.print("B:             ");
  printBinary8(B[test]);
  Serial.print("  (");
  Serial.print(B[test]);
  Serial.println(")");

  /*
    Print the expected state of all nine LEDs:

    COUT | S7 S6 S5 S4 S3 S2 S1 S0
  */
  Serial.print("EXPECTED LEDs: ");
  Serial.print(expectedCarry);
  Serial.print(" ");
  printBinary8(expectedResult);
  Serial.println();

  Serial.println("               COUT S7-S0");

  if (subtractMode[test]) {
    Serial.print("BORROW:        ");

    if (expectedCarry) {
      Serial.println("NO BORROW");
    }
    else {
      Serial.println("BORROW OCCURRED");
    }

    if (bitRead(expectedResult, 7)) {
      Serial.println(
        "SIGNED RESULT: Negative two's complement"
      );
    }
  }
  else {
    Serial.print("CARRY-OUT:     ");

    if (expectedCarry) {
      Serial.println("CARRY OCCURRED");
    }
    else {
      Serial.println("NO CARRY");
    }
  }

  Serial.println("Visually check all nine LEDs.");
  Serial.println("----------------------------");
}


// Check whether STOP was entered.
bool stopRequested() {
  if (Serial.available() == 0) {
    return false;
  }

  String command =
    Serial.readStringUntil('\n');

  command.trim();
  command.toUpperCase();

  if (command == "STOP") {
    running = false;

    Serial.println();
    Serial.println("TEST SEQUENCE STOPPED");
    Serial.println("Type START to run again.");

    return true;
  }

  return false;
}


void setup() {
  Serial.begin(9600);

  // Configure pins 3–10 as bus outputs.
  for (byte bit = 0; bit < 8; bit++) {
    pinMode(dataPins[bit], OUTPUT);
  }

  pinMode(latchPin, OUTPUT);
  pinMode(subtractPin, OUTPUT);

  digitalWrite(latchPin, LOW);
  digitalWrite(subtractPin, LOW);
  writeBus(0);

  Serial.println();
  Serial.println("8-BIT ADDER/SUBTRACTOR READY");
  Serial.println("Type START to begin.");
  Serial.println("Type STOP to stop.");
}


void loop() {
  // Wait for a Serial Monitor command.
  if (Serial.available() > 0) {
    String command =
      Serial.readStringUntil('\n');

    command.trim();
    command.toUpperCase();

    if (command == "START") {
      running = true;

      Serial.println();
      Serial.println("TEST SEQUENCE STARTED");
    }
    else if (command == "STOP") {
      running = false;

      Serial.println();
      Serial.println("TEST SEQUENCE STOPPED");
    }
    else {
      Serial.println(
        "Unknown command. Type START or STOP."
      );
    }
  }

  // Run all five tests after START.
  if (running) {
    for (byte test = 0; test < 5; test++) {
      runTest(test);

      /*
        Hold the result for seven seconds.

        Check for STOP once every second.
      */
      for (byte second = 0; second < 7; second++) {
        delay(1000);

        if (stopRequested()) {
          return;
        }
      }
    }

    running = false;

    Serial.println();
    Serial.println("ALL FIVE TESTS COMPLETED");
    Serial.println("Type START to run them again.");
  }
}