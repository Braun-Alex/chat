<template>
  <q-layout view="hHh lpR fFf">
    <q-page-container>

      <q-page padding>
        <div class="flex flex-center full-height">
          <div v-if="!isRegistered" class="q-pa-md">
            <q-input outlined v-model="username" label="Нікнейм" class="q-mb-md" />
            <q-btn rounded @click="register" glossy color="primary" no-caps label="Приєднатися" class="full-width" />
          </div>

          <div v-if="isRegistered" class="q-pa-md">
            <q-list separator class="q-mb-md chat-history">
              <q-item v-for="(msg, index) in messages" :key="index">
                <q-item-section>
                  <q-card :class="isMyMessage(msg)? 'chat-message my-message q-pa-sm q-mb-sm': 'chat-message q-pa-sm q-mb-sm'">
                    <q-card-section>
                      {{ getMessageText(msg) }}
                    </q-card-section>
                  </q-card>
                </q-item-section>
              </q-item>

            </q-list>

            <q-input outlined v-model="message" label="Повідомлення" class="q-mb-md" />

            <div class="row items-center q-mb-md">
              <div class="col">
                <q-input :disable="!privateMode" outlined v-model="target" label="Нікнейм отримувача" />
              </div>
              <div class="col-2">
                <q-toggle icon="lock" v-model="privateMode"
                          :label="privateMode ? 'Приватний режим': 'Широкомовний режим'" />
              </div>
            </div>

            <q-btn @click="sendMessage" glossy color="primary" no-caps rounded :disable="privateMode && target.length == 0"
                   label="Надіслати" class="full-width" />
            <q-btn @click="disconnect" glossy color="negative" no-caps rounded label="Вийти з чату" class="full-width q-mt-sm" />
          </div>
        </div>
      </q-page>

    </q-page-container>
  </q-layout>
</template>

<script setup lang="ts">
import { ref } from 'vue';

const username = ref<string>('');
const message = ref<string>('');
const target = ref<string>('');
const isRegistered = ref<boolean>(false);
const privateMode = ref<boolean>(false);
const socket = ref<WebSocket | null>(null);
const messages = ref<string[]>([]);

function register() {
  socket.value = new WebSocket('ws://localhost:3000');
  socket.value.onopen = () => {
    socket.value?.send('CONNECT: ' + username.value);
    isRegistered.value = true;
    window.onclose = () => {
      disconnect();
    }
  }

  socket.value.onmessage = (event: MessageEvent) => {
    const receivedMessages = (event.data as string).split('\0');
    for (const msg of receivedMessages) {
      if (msg.length != 0) {
        messages.value.push(msg);
      }
    }
  }
}

function sendMessage() {
  if (privateMode.value) {
    socket.value?.send('PRIVATE MESSAGE: ' + target.value + ': ' + message.value);
  } else {
    socket.value?.send('PUBLIC MESSAGE: ' + message.value);
  }
  message.value = '';
}

function disconnect() {
  socket.value?.send('DISCONNECT: ' + username.value);
  socket.value?.close();
  isRegistered.value = false;
  username.value = '';
}

function isMyMessage(msg: string) {
  if (msg.startsWith('[Приватно до ')) {
    return true;
  }
  if (!msg.includes(':')) {
    return false;
  }
  return msg.split(': ')[0].trim() === username.value;
}

function getMessageText(msg: string) {
  if (!msg.includes(':') || !isMyMessage(msg)) return msg;
  return msg.split(': ').slice(1).join(': ').trim();
}
</script>

<style scoped>
.chat-history {
  max-height: 400px;
  overflow-y: auto;
}

.chat-message {
  max-width: 70%;
  border-radius: 15px;
  background-color: #e0e0e0;
}

.my-message {
  background-color: #4caf50;
  color: white;
  margin-left: auto;
}
</style>


