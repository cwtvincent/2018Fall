import keras
from keras.datasets import mnist
from keras.models import Sequential
from keras.layers import Dense
from matplotlib import pyplot as plt

(x_train, y_train), (x_test, y_test) = mnist.load_data()
x_train_2D = x_train.reshape(60000, 28*28).astype('uint8')
x_test_2D = x_test.reshape(10000, 28*28).astype('uint8')
y_trainonehot = keras.utils.to_categorical(y_train) 
y_testonehot = keras.utils.to_categorical(y_test) 

model = Sequential()
model.add(Dense(units=196, 
                bias_initializer='normal',
                input_dim=784, 
                kernel_initializer='normal', 
                activation='relu'))
model.add(Dense(units=49, 
                bias_initializer='normal',
                kernel_initializer='normal', 
                activation='relu'))
model.add(Dense(units=10, 
                bias_initializer='normal',
                kernel_initializer='normal', 
                activation='softmax'))

model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy']) 

hist = model.fit(x=x_train_2D, 
                 y=y_trainonehot, 
                 validation_split=0.2, 
                 epochs=10, 
                 batch_size=800, 
                 verbose=2) 

scores = model.evaluate(x_test_2D, y_testonehot)  
print()  
print("Accuracy of testing data = {:2.1f}%".format(scores[1]*100.0))

plt.plot(hist.history['loss'])  
plt.plot(hist.history['val_loss'])  
plt.title('Train History')  
plt.ylabel('loss')  
plt.xlabel('Epoch')  
plt.legend(['loss', 'val_loss'], loc='upper right')  
plt.show() 

plt.plot(hist.history['acc'])  
plt.plot(hist.history['val_acc'])  
plt.title('Train History')  
plt.ylabel('acc')  
plt.xlabel('Epoch')  
plt.legend(['acc', 'val_acc'], loc='lower right')  
plt.show() 

l2weight = model.layers[2].get_weights()[0]
l2bias = model.layers[2].get_weights()[1]
