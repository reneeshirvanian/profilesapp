import { defineFunction } from '@aws-amplify/backend';

export const sendAlert = defineFunction({
  name: 'send-alert',
  entry: './handler.ts',
});
