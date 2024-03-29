import pandas as pd
from sklearn.discriminant_analysis import StandardScaler
from sklearn.model_selection import train_test_split
import controlschemes


CONTROL_SCHEME = controlschemes.page
DEVICE_LOCATION = '/dev/tty.usbserial-02896C6F'
MODEL_PATH = 'src/ml/models/model.h5'
NUM_POSSIBILITIES = len(CONTROL_SCHEME)
BAUD_RATE = -1
NUM_SENSORS = -1
NUM_SAMPLES = -1
NUM_EPOCHS = 100

#Find what they have been initialized to in constants.h. Don't want to have to change them in 2 places.
with open("src/constants.h") as constantsH:
    for line in constantsH.readlines():
        if "const static int SAMPLES_PER_ACTION" in line:
            NUM_SAMPLES = int(line.split()[-1][:-1])
        elif "const static int NUM_SENSORS" in line:
            NUM_SENSORS = int(line.split()[-1][:-1])
        elif "static const int BAUD_RATE" in line:
            BAUD_RATE = int(line.split()[-1][:-1])

ONLYPRINT = False 

# if actual training data exists
if sum(1 for _ in open('src/ml/csvs/mpu6050data.csv')) > 1:
    scaler = StandardScaler()
    df = pd.read_csv('src/ml/csvs/mpu6050data.csv')
    X = df.drop('key', axis=1)
    y = df['key']
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, 
        test_size=0.2, random_state=2
    )
    scaler.fit_transform(X_test)
