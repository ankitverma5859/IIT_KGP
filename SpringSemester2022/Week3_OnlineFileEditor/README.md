Description of the Application:
This clien-server application enables clients to upload files to the server, invite other clients for collaboration with View(V) or Editor(E) permission.
The client/owners with their respective permission will be able to read, insert, and delete contents of the files.
Commands /users, /files helps user to get information for the active users and files along with there metadata respectively.


File Structure

/21CS60A04_CL2_A3
	server.c
	client.c
	/client_files  		           			
		
		/downloaded					-------->"Directory where the files downloaded from the server reside."
		
		test.txt					-------->"Files to be uploaded to server should reside here. Also files should have read permission."
		test1.txt
	/server_files
		
		/client_uploads             -------->"All the files uploaded by the cliend reside at this directory"
			test.txt
			test1.txt
		
		client.txt                  -------->"File used by the server to keep record of the clients"
		collaborators.txt           -------->"File used by ther server to keep record of the collaborators"
		files_metadata.txt    		-------->"File used by the server to keep the metadata of the uploaded files"
		invitations.txt				-------->"File used by the server to keep track the invitations."


How to run the application:

STEP 1: Change your directory to 21CS60A04_CL2_A3/

STEP 2: Compile the server.c file
		gcc -o server server.c

STEP 3: Compile the client.c file
		gcc -o client client.c

STEP 4: Run the server
		./server <PORT_NUMBER>
		Example: ./server 9000
		The application will throw ERROR if the format of the command is incorrect.

STEP 5: RUN the client
		./client <SERVER_IP> <SERVER_PORT>
		Example: ./client localhost 9000

		[Step 1 - 5] -> Screeshot_1

STEP 6: Upload a file
		/upload <filename>
		Example: /upload test.txt
				 * test.txt should be present in client_files directory

		[Step 6] -> Screenshot_2

STEP 7: Download a file
		/download <filename>
		Example: /download test.txt

		[Step 7] -> Screenshot_3

STEP 8: Read from a file.
		/read <filename>								Reads the whole file
		/read <filename>	<index>						Reads the index of the file
		/read <filename>	<start_index>	<end_index> Read from the start_index to end_index


		[Step 8] -> Screenshot_4, Screenshot_5

STEP 9: Insert into a file
		/insert <filename> "<message>"					Inserts at the end of the file
		/insert <filename> <index> "<message>"			Inserts at the <index> of the file

		[Step 9] -> Screenshot_6

STEP 10:Delete from a file
		/delete <filename>								Deletes all the contents of the file
		/delete <filename>	<index>						Deletes the line at the <index>
		/delete <filename>	<start_index>	<end_index> Deletes the lines from <start_index> <end_index>

		[Step 10] -> Screenshot_7

STEP 11:Active users
		/users

		[Step 11] -> Screnshot_8

STEP 12:Invite for a collaboration
		(i)	Open one more terminal with client instance for second client.
			You will be greeted with 5-digit unique Id.

		(ii)Go to the terminal of first client now. And enter the below command.
			/invite <filename> <5-digit-client-id> <permission>
			Example: /invite test.txt 12345 V
			*acceptable values of permission are V/E

			Now, go to the terminal of second client(12345).
			Since, the client 2 is opened with enter the command. It will show notifications post any valid command for the client.
		(iii)Example: /users
			You will be prompted with pending invitations.
		(iv)Enter (Y/N) to accept or reject the invitation.

		If you accepted. Now you will be able to download, read, insert, delete based upon the permission provided

		[Step 12] -> Screeshot_9

STEP 13:Files uploaded
		/files

		[Step 13] -> Screensho_10

STEP 14:Exit
		/exit

		[Step 14] -> Screenshot_10

		*CTRL + C is also handled for graceful closing/termination of the application
