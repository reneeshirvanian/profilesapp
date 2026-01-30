import { SNSClient, PublishCommand } from "@aws-sdk/client-sns";

const sns = new SNSClient({});
// ⚠️ REPLACE THIS with your actual Topic ARN from AWS Console ⚠️
const TOPIC_ARN = "arn:aws:sns:us-east-1:123456789012:PillDispenserAlerts";

export const handler = async (event) => {
  try {
    const command = new PublishCommand({
      TopicArn: TOPIC_ARN,
      Message: "TIME TO TAKE YOUR MEDICATION! 💊",
      Subject: "Pill Alert",
    });

    await sns.send(command);
    return { success: true };
  } catch (error) {
    console.error("SNS Error:", error);
    return { success: false, error: error.message };
  }
};
