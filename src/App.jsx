import { useState, useEffect } from "react";
import {
  Button,
  Heading,
  Flex,
  View,
  Divider,
} from "@aws-amplify/ui-react";
import { useAuthenticator } from "@aws-amplify/ui-react";
// 🛑 IMPORTANT FIX: Removed 'Auth' from this import to fix the SyntaxError
import { Amplify } from "aws-amplify"; 
import "@aws-amplify/ui-react/styles.css";
import { generateClient } from "aws-amplify/data";
import outputs from "../amplify_outputs.json";

/**
 * @type {import('aws-amplify/data').Client<import('../amplify/data/resource').Schema>}
 */
Amplify.configure(outputs);

const client = generateClient({
  authMode: "userPool",
});

export default function App() {
  // We extract 'user' and 'signOut' from the context
  const { user, signOut } = useAuthenticator((context) => [context.user]);
  
  // New State for Schedules and Form Inputs
  const [schedules, setSchedules] = useState([]);
  const [medicationName, setMedicationName] = useState('');
  const [dosageTime, setDosageTime] = useState('');
  const [loading, setLoading] = useState(true);

  // Get email from the Authenticator context
  const userEmail = user.attributes?.email || 'N/A';
  // Get the user's unique ID (sub) directly from the hook's user object
  const profileOwnerId = user.userId; 

  useEffect(() => {
    // Only fetch schedules if the user object (and thus ID) is available
    if (profileOwnerId) {
        fetchSchedules(profileOwnerId);
    }
  }, [profileOwnerId]); // Depend on the ID, not the whole user object

  // Function to fetch schedules for the CURRENTLY logged-in user
  async function fetchSchedules(ownerId) {
    try {
      setLoading(true);
      
      // We use the ID directly from the hook, avoiding the problematic Auth import
      const { data: scheduleData } = await client.models.MedicationSchedule.list({
        // Filter by the current user's ID
        filter: { profileOwner: { eq: ownerId } },
      });
      
      setSchedules(scheduleData);
    } catch (error) {
      console.error('Error fetching schedules:', error);
    } finally {
      setLoading(false);
    }
  }

  // Function to handle adding a new schedule
  const handleAddSchedule = async (e) => {
    e.preventDefault();
    if (!medicationName.trim() || !dosageTime.trim() || !profileOwnerId) return;

    try {
      // Create the new schedule record in the database
      await client.models.MedicationSchedule.create({
        name: medicationName,
        time: dosageTime,
        // We use the ID directly from the hook
        profileOwner: profileOwnerId, 
      });

      // Clear form and refresh the list
      setMedicationName('');
      setDosageTime('');
      fetchSchedules(profileOwnerId); // Fetch the updated list
    } catch (error) {
      console.error('Error creating schedule:', error);
    }
  };

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

      <Flex direction="column" alignItems="center" margin="1rem 0">
        <Heading level={3}>Welcome, {userEmail}</Heading>
      </Flex>
      <Divider />

      {/* 💊 MEDICATION SCHEDULE INPUT FORM */}
      <Heading level={2} margin="2rem 0 1rem">💊 Add Medication Schedule</Heading>
      <form onSubmit={handleAddSchedule} style={{ display: 'flex', gap: '10px', width: '100%', maxWidth: '600px', marginBottom: '30px' }}>
        <input
          type="text"
          placeholder="Medication Name (e.g., Tylenol)"
          value={medicationName}
          onChange={(e) => setMedicationName(e.target.value)}
          required
          style={{ padding: '8px', flexGrow: 2 }}
        />
        <input
          type="text"
          placeholder="Time (e.g., Mon 8:00 AM)"
          value={dosageTime}
          onChange={(e) => setDosageTime(e.target.value)}
          required
          style={{ padding: '8px', flexGrow: 1 }}
        />
        <Button type="submit" variation="primary">Add Schedule</Button>
      </form>
      <Divider />

      {/* 🗓️ CURRENT SCHEDULE DISPLAY */}
      <Heading level={2} margin="2rem 0 1rem">🗓️ My Current Schedules</Heading>
      {loading ? (
        <p>Loading schedules...</p>
      ) : schedules.length === 0 ? (
        <p>No medications scheduled yet. Use the form above to add one!</p>
      ) : (
        <View style={{ width: '100%', maxWidth: '600px' }}>
          {schedules.map((schedule) => (
            <Flex
              key={schedule.id}
              justifyContent="space-between"
              alignItems="center"
              padding="10px"
              marginBottom="5px"
              border="1px solid #ddd"
              borderRadius="5px"
            >
              <strong>{schedule.name}</strong> 
              <span>Scheduled for: **{schedule.time}**</span>
            </Flex>
          ))}
        </View>
      )}

      <Divider margin="2rem 0" />
      <Button onClick={signOut} variation="warning">Sign Out</Button>
    </Flex>
  );
}