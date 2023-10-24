<h2>Install ESP8266 SDK from ExpressIf</h2>

Per https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/windows-setup.html

1. Download

    https://dl.espressif.com/dl/esp32_win32_msys2_environment_and_toolchain-20181001.zip

2. Unzip it to dedicated folder (d:/esp), this will create a "msys32" folder

3. Confirm that MinGW works.  Run following from a cmd line:

        > d:\esp\msys32\mingw32

4. Download chip specific compiler:

    https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-win32.zip

5. Unzip this into \esp\msys32\opt (next to other chip compiler)

6. Update the msys32/etc/profile.d/esp32_toolchain.sh with additional compiler

        export PATH="$PATH:/opt/xtensa-esp32-elf/bin:/opt/xtensa-lx106-elf/bin"

7. Clone the SDK:

        > cd d:\esp
        > git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git

<h2>Create desktop enviroment</h2>

1. Create an environment variables setvars.cmd file that (1) adds to path, and (2) defines the IDF_PATH

        set IDF_PATH=D:\ESP\ESP8266_RTOS_SDK

2. Create desktop shortcut for opening environment:

        %comspec% /k D:\ESP\setvars.cmd

3. Reopen environment with the new shortcut


<h2>Install the ESP8266 SDK's requirements</h2>

1. In the cmd window:

        > msys32\mingw32\bin\python2.7 -m  pip install --user -r %IDF_PATH%/requirements.txt

   
    >Warning: The Click and Pyelftools land in the C:\Users\<username>\.local\lib\python2.7\site-packages directory.  Just move to D:\ESP\msys32\mingw32\lib\python2.7\site-packages 


2. Create an "apps" folder with a "hello_world" subfolder, and copy all of follow folder into it:

       %IDF_PATH%\examples\get-started\hello_world

3. Open shell with
   
        d:\esp\msys32\mingw32

4. Change to hello_world folder.  

        cd /d/esp/apps/hello_world

5. Run:
 
        make menuconfig

6. Set "Serial flasher config" > "Default serial port" to COM11

7. Execute one of the following make files: 

        make flash
        make monitor

        or

        make flash monitor

    >This will take awhile

8. Confirm that code was flashed to the chip



<h2>Install SmartThings SDK</h2>

> https://github.com/SmartThingsCommunity/st-device-sdk-c-ref



1. With Windows cmd line:

        > cd d:\esp
        > git clone https://github.com/SmartThingsCommunity/st-device-sdk-c-ref.git

> Optional - fork the above repository so that you can incorporate your own changes

2. It appears that MinGN (a) doesn't handle path translation correctly, and (b) doesn't actually run the .sh script.
   
   Added following snippet to both setup.py and build.py near the end of the python script:

        `if os.name == "nt":
                setup_cmd = setup_cmd.replace('/', '\\')`

2. Pull in addition chip-specific dependencies (run in shell):

        > \esp\msys32\mingw32  (opens shell)
        > cd /d/esp/st-device-sdk-c-ref
        $ python setup.py      (not sure if this is necessary)
        $ python setup.py esp8266

    This clones the "iot-core" for specific chip

3. Manually run the setup*.sh displayed in the updated setup.py 

        tools/esp8266/setup_esp8266.sh esp8266

4. Continue on with the build of example app:

        python build.py apps/esp8266/light_example

5. Manually run the build*.sh displayed in the updated build.py

        tools/esp8266/build_esp8266.sh esp8266 light_example flash monitor
        or
        tools/esp8266/build_esp8266.sh esp8266 light_sensor flash monitor

6. Update skdconfig in app directory either manually:
   
        CONFIG_ESPTOOLPY_PORT="COM11"

   Or by running

        python build.py esp8266 light_example menuconfig

7. Confirm that it built without errors.



<h2>Configuring Application</h2>


1. Go to SmartThings Developer Workspace at >https://smartthings.developer.samsung.com/workspace/projects and proceed to the area for creating a "direct-connected" device project (New Project > Device Integration > Direct-connected)

    - Enter Project Name
    - Define Device Profile (and add appropriate capabilities)
    - Define new Device Onboarding (add onboarding to project)
    - Enter Project Info
    - Deploy to Test

>From "Overview" download the Onboarding_config.json


2. In a windows command line (install Python 3+ if necessary):

        > cd iot-core\tools\keygen
        > pip install pynacl   (for signing library)
        > python stdk-keygen.py --firmware <app name>

   This will (1) generate keys, and (2) create a device_info.json file in a subdirectory.

        Serial Number:
        STDKpDEkCI0xJccr

        Public Key:
        HtqHMxc7T7EMme3dyc7f3f/VKuNOPVJoiv8CLPi6Sus=

3. Return to the Developer Workspace and register a new device by entering the serial number and public key from above.


<h2>Build Registerable App</h2>

1. Replace the onboarding_config.json in the application's "main" folder with the json file downloaded from website.

2. Replace the device_info.json in the application's "main" folder with the json file in the ..\iot-core\tools\keygen\output_* folder.

3. Rebuild application


<h2>Create QR Code for Registering Device</h2>

1. Open a python window

2. Execute the following commands (pip install qrcode, if necessary):

        > import qrcode
        > qrURL = 'https://qr.samsungiots.com/?m=fm1I&s=001&r=STDKGIdw65hjd3gZ'
        > qrcode.QRCode(box_size=10, border=4)
        > img = qrcode.make(qrURL)
        > img.save('myqr.png')
        > exit()

   The format of the Url is as follows:

        https://qr.samsungiots.com/?m={Your mnId}&s={Device onboardingId}&r={Device serialNumber}

   The mnId and onboardingID can be found in the onboarding_config.json.  The serialNumber is from keygen above or the device_info.json file.

3. Add new device to your SmartThings account
   - "Add Device"
   - scan your QR code



<h2>Other tools</h2>

to clear chip:

        python -m esptool --chip esp8266 erase_flash