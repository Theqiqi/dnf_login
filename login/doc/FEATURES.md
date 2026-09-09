# DNF Taiwan Launcher - Core Feature Technical Specifications

This section breaks down the data flow pipelines, validation sequences, and database tracking behaviors for the four mandatory operational workflows within the custom runtime launcher.

```apl
       [ CLI / Future UI Entry Router ]
                      │
     ┌────────────────┼────────────────┬────────────────┐
     ▼                ▼                ▼                ▼
[1. Register]   [2. Name Login]   [3. UID Login]  [4. Vector Check]
     │                │                │                │
(d_taiwan.      (d_taiwan.       (taiwan_login.   (taiwan_login.
 accounts)       accounts)        member_login)    member_login)
```

------

## 1. User Registration Routine (Account Provisioning)

Allows unauthenticated terminal nodes to inject fresh credential blocks directly into the server cluster.

A. Architectural Sequence Flow

1. **Inputs Collected**: Captures plain ASCII `accountname` and standard `password` strings via the app routing view.

2. **Sanitization Filter**: Passes input buffers into `Database::escapeString` to automatically strip toxic patterns and prevent SQL Injection attacks.

3. **Cryptographic Hashing**: Converts the sanitized plain-text password into a 32-character lowercase hexadecimal MD5 hash signature via `Database::md5Hex`.

4. **Database Transaction**: Compiles and executes an explicit cross-database insertion query targeting the primary auth node:

  

   ```  sql
   INSERT INTO d_taiwan.accounts (accountname, password) VALUES ('<safe_name>', '<md5_hash>');
   ```

   Use code with caution.

5. **Autoincrement Hook**: Catches the response context, invokes `mysql_insert_id`, extracts the database-allocated primary integer `UID`, and maps it back into the user context as proof of account assignment.

------

## 2. Standard Credentials Authentication (Login by Username)

Validates the credentials mask and dynamically extracts the underlying Account ID required to start the cryptographic game token generation sequence.

A. Architectural Sequence Flow

```apl
[User Input] ──> [Sanitize & MD5] ──> [Query d_taiwan.accounts]
                                                │
                 ┌──────────────────────────────┴──────────────────────────────┐
                 ▼ (Account Found + PW Match)                                  ▼ (No Match)
       [Fetch Field[0] as UID]                                             [Reject Session]
                 │
                 ▼
       [Execute Game Launch]
```

B. Execution Sequence Steps

1. Captures `username` and `password` string properties, running identical SQL escaping filters and MD5 hexadecimal digest generations.

2. Queries the core account tracking matrix explicitly specifying both authentication keys:

   

   ```sql
   SELECT * FROM d_taiwan.accounts WHERE accountname='<safe_name>' AND password='<md5_hash>';
   ```

   Use code with caution.

   

   

3. Evaluates the server results via `mysql_num_rows`. If zero rows hit, the subsystem drops the state and fires an `Authentication Rejected` alert.

4. If a record hits, `mysql_fetch_row` extracts the first index pointer (`row[0]`) representing the system account `UID` (e.g., `"12345"`).

5. Pass the extracted `UID` downstream directly into the `executeGameLaunch` pipeline.

------

## 3. Direct Account Bypass (Login by UID)

Bypasses standard credential matching by directly loading a target user ID matrix. This mimics the administrative "quick login" function of your original reverse-engineered binaries, verifying that the target account is actively whitelisted on the server.

A. Architectural Sequence Flow

```apl
[Input User UID] ──> [Query taiwan_login.member_login]
                                     │
                 ┌───────────────────┴───────────────────┐
                 ▼ (Record Exists)                       ▼ (Record Miss)
       [Bypass Approved]                               [Reject Session]
                 │                               "UID Not Activated"
                 ▼
       [Execute Game Launch]
```

B. Execution Sequence Steps

1. Receives a direct numerical `UID` string from the user prompt.

2. Processes the parameter through `Database::escapeString` to isolate memory segments.

3. Queries the activation and runtime tracking database to ensure the selected account has a verified server activation status:

   

   ```sql
   SELECT * FROM taiwan_login.member_login WHERE m_id=<safe_uid>;
   ```

   Use code with caution.

   

4. Verifies the query output data layer. If the account profile is missing from `member_login`, the terminal throws an error specifying that the targeted UID is not registered or activated.

5. If the record successfully returns a valid row context, the launcher grants full authorization and transfers execution to `executeGameLaunch`.

------

## 4. Account Validation Node (Vector Check / UID Query)

A low-privilege administrative diagnostic tool designed to quickly scan the network cluster and verify account presence without initiating a client launch.

A. Architectural Sequence Flow

1. Receives an arbitrary target `UID` value string.
2. Internally maps directly to the underlying `Database::loginByUid` boolean checking engine.
3. Checks the state return code:
   - **`True` (Vector Hit)**: Confirms the account is registered, active, and fully cleared on the server cluster.
   - **`False` (Vector Miss)**: Confirms the selected identifier does not exist within the server tracking memory maps.
4. Outputs the safe diagnostic analysis directly into the console stream without allocating any downstream execution runtime handles.

------

## 5. Master Session Processing Pipeline (`executeGameLaunch`)

Once any authorization vector (Username or Direct UID) succeeds, it calls this master routine to perform the RSA signing operations and securely hand off control to the operating system.

```apl
 [ Authorized Integer UID ]
              │
              ▼
    [ Ticket Generation ]    ──> Runs Ticket::CreateRawPacket() 
              │                  Outputs exact 46-byte memory vector.
              ▼
    [ Cryptographic Seal ]   ──> Enforces OpenSSL RSA_PKCS1_PADDING
              │                  Bakes the 1FFFFF...00 header automatically.
              ▼                  Outputs a standard 256-byte signature block.
    [ Base64 Streaming ]     ──> Encodes the signature to ASCII Base64 Token.
              │
              ▼
    [ Win32 Safe Launch ]    ──> Allocates mutable CLI buffer.
                                 Forces absolute working directory context.
                                 Invokes CreateProcessA("dnf.exe", Token).
```

------