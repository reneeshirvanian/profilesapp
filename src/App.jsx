import { useState, useEffect } from "react";
import {
  Button,
  Heading,
  Flex,
  View,
  Grid,
  Divider,
} from "@aws-amplify/ui-react";
import { useAuthenticator } from "@aws-amplify/ui-react";
import { Amplify } from "aws-amplify";
import "@aws-amplify/ui-react/styles.css";
import { generateClient } from "aws-amplify/data";
import outputs from "../amplify_outputs.json";

/**
 * @type {import('aws-amplify/data').Client<import('../amplify/data/resource').Schema>}
 */
Amplify.configure(outputs);

// Data client using Cognito user pools
const client = generateClient({
  authMode: "userPool",
});

export default function App() {
  const [userprofiles, setUserProfiles] = useState([]);
  const [schedules, setSchedules] = useState([]);
  const [medicationName, setMedicationName] = useState("");
  const [dosageTime, setDosageTime] = useState("");
  const [loading, setLoading] = useState(true);

  // user and signOut from Amplify Authenticator
  const { user, signOut } = useAuthenticator((context) => [context.user]);

  // Email + userId from Cognito user
  const username = user?.username ?? user?.attributes?.email ?? "User";
  const userId = user?.userId; // Cognito sub

  // Fetch profiles once
  useEffect(() => {
    fetchUserProfile();
  }, []);

  // Fetch schedules when user is ready
  useEffect(() => {
    if (userId) {
      fetchSchedules(userId);
    }
  }, [userId]);

  async function fetchUserProfile() {
    const { data: profiles } = await client.models.UserProfile.list();
    setUserProfiles(profiles);
  }

  // Fetch schedules that belong to the current user
  async function fetchSchedules(ownerId) {
    try {
      setLoading(true);
      const { data: scheduleData } = await client.models.MedicationSchedule.list({
        filter: { profileOwner: { eq: ownerId } },
      });
      setSchedules(scheduleData);
    } catch (error) {
      console.error("Error fetching schedules:", error);
    } finally {
      setLoading(false);
    }
  }

  // Add a new schedule for this user
  async function handleAddSchedule(e) {
    e.preventDefault();
    if (!medicationName.trim() || !dosageTime.trim() || !userId) return;

    try {
      await client.models.MedicationSchedule.create({
        name: medicationName,
        time: dosageTime,
        profileOwner: userId,
      });

      setMedicationName("");
      setDosageTime("");
      fetchSchedules(userId);
    } catch (error) {
      console.error("Error creating schedule:", error);
    }
  }

  return (
    <Flex
      className="App"
      justifyContent="center"
      alignItems="center"
      direction="column"
      width="70%"
      margin="0 auto"
    >
      <Heading level={1}>My Profile</Heading>
      <Divider />

      {/* Existing profile cards */}
      <Grid
        margin="3rem 0"
        autoFlow="column"
        justifyContent="center"
        gap="2rem"
        alignContent="center"
      >
        {userprofiles.map((userprofile) => (
          <Flex
            key={userprofile.id || userprofile.email}
            direction="column"
            justifyContent="center"
            alignItems="center"
            gap="2rem"
            border="1px solid #ccc"
            padding="2rem"
            borderRadius="5%"
            className="box"
          >
            <View>
              <Heading level={3}>{userprofile.email}</Heading>
            </View>
          </Flex>
        ))}
      </Grid>

      {/* Current user info */}
      <Heading level={3}>Welcome, {username}</Heading>

      <Divider margin="2rem 0 1rem" />

      {/* 💊 Add Medication Schedule */}
      <Heading level={2} margin="0 0 1rem">
        💊 Add Medication Schedule
      </Heading>

      <form
        onSubmit={handleAddSchedule}
        style={{
          display: "flex",
          gap: "10px",
          width: "100%",
          maxWidth: "600px",
          marginBottom: "30px",
        }}
      >
        <input
          type="text"
          placeholder="Medication Name (e.g., Tylenol)"
          value={medicationName}
          onChange={(e) => setMedicationName(e.target.value)}
          required
          style={{ padding: "8px", flexGrow: 2 }}
        />
        <input
          type="text"
          placeholder="Time (e.g., Mon 8:00 AM)"
          value={dosageTime}
          onChange={(e) => setDosageTime(e.target.value)}
          required
          style={{ padding: "8px", flexGrow: 1 }}
        />
        <Button type="submit" variation="primary">
          Add Schedule
        </Button>
      </form>

      <Divider margin="2rem 0 1rem" />

      {/* 🗓️ Display Schedules */}
      <Heading level={2} margin="0 0 1rem">
        🗓️ My Current Schedules
      </Heading>

      {loading ? (
        <p>Loading schedules...</p>
      ) : schedules.length === 0 ? (
        <p>No medications scheduled yet. Use the form above to add one!</p>
      ) : (
        <View style={{ width: "100%", maxWidth: "600px" }}>
          {schedules.map((schedule) => (
            <Flex
              key={schedule.id}
              justifyContent="space-between"
              alignItems="center"
              padding="12px"
              marginBottom="10px"
              border="1px solid #ddd"
              borderRadius="8px"
              backgroundColor="#f9f9f9"
              boxShadow="0 1px 3px rgba(0,0,0,0.1)"

            >
              <strong style={{ color: "#000" }}>{schedule.name}</strong>
              <span style={{ color: "#333" }}>Scheduled for: {schedule.time}</span>
            </Flex>
          ))}
        </View>
      )}

      <Divider margin="2rem 0" />
      <Button onClick={signOut} variation="warning">
        Sign Out
      </Button>
    </Flex>
  );
}
