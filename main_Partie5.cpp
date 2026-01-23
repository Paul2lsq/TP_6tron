#include "mbed.h"
#include <nsapi_dns.h>
#include <MQTTClientMbedOs.h>
#include "bme280.h"

// Définition des noms d'utilisateur et des topics pour Adafruit IO
namespace {
    #define AIO_USERNAME "Paul2sq"
    #define FEED_TEMP   AIO_USERNAME "/feeds/temperature"
    #define FEED_HUM    AIO_USERNAME "/feeds/humidity"
    #define FEED_PRESS  AIO_USERNAME "/feeds/pression"
}

// Déclaration des objets utilisés
static DigitalOut led(LED1);                 // LED pour l'affichage
static InterruptIn button(BUTTON1);          // Bouton pour déclencher une action

I2C i2c(I2C1_SDA, I2C1_SCL);                // Bus I2C pour communiquer avec le capteur BME280
sixtron::BME280 sensor(&i2c);               // Objet capteur BME280

EventQueue queue(32 * EVENTS_EVENT_SIZE);    // File d'événements pour la gestion des tâches
Thread queue_thread;                         // Thread pour exécuter la file d'événements

NetworkInterface *network;                   // Interface réseau pour la connexion
MQTTClient *client;                          // Client MQTT pour envoyer les données

// Variables globales pour les données du capteur
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

// Mutex pour protéger l'accès aux variables partagées
Mutex data_mutex;
Mutex stdio_mutex;

// Configuration du serveur MQTT
const char* hostname = "io.adafruit.com";
int port = 1883;
nsapi_size_or_error_t rc = 0;

// Déclaration des threads
Thread sensorThread;
Thread mqttThread;
Thread ledThread;

// Fonction pour faire clignoter la LED
void blink_led() {
    while (true) {
        led = !led;  // Change l'état de la LED
        ThisThread::sleep_for(500ms);  // Attente de 500ms
    }
}

// Fonction pour lire les données du capteur
void sensor_thread() {
    while (true) {
        // Verrouille les données pour éviter les accès simultanés
        data_mutex.lock();
        temperature = sensor.temperature();
        humidity = sensor.humidity();
        pressure = sensor.pressure() / 100;  // Conversion en hPa
        data_mutex.unlock();

        // Si la température dépasse 26°C, on allume la LED
        if (temperature > 25.5) {
            
                ledThread.start(blink_led);  // Démarre le thread pour clignoter la LED
            }
    

        ThisThread::sleep_for(5s);  // Attente de 5 secondes avant de refaire une lecture
    }
}

// Fonction pour imprimer les données quand le bouton est pressé
void task_print() {
    float t = temperature;
    float h = humidity;
    float p = pressure;

    // Affiche les valeurs des capteurs
    printf("Temperature : %f C\n", t);
    printf("Humidite    : %f %%\n", h);
    printf("Pression    : %f Pa\n", p);

    ThisThread::sleep_for(1s);  // Attend 1 seconde
    //led = 0;  // Éteint la LED

    if (humidity < 40) {
        int x = 0;
        printf(" \n \n Arrosez votre geranium monsieur/madame !!!!! \n");
        while (x++ == 10) {
        led = !led;  // Change l'état de la LED
        ThisThread::sleep_for(500ms);  // Attente de 500ms
    }
    }

    if (temperature > 26) {
        int x = 0;
        printf(" \n \n Un petit coup de frais pour votre geranium monsieur/madame !!!!! \n");
        while (x++ == 10) {
        led = !led;  // Change l'état de la LED
        ThisThread::sleep_for(500ms);  // Attente de 500ms
    }
    }

}

// Fonction appelée quand le bouton est pressé
void on_button_rise() {
    queue.call(task_print);  // Appelle la fonction task_print dans le thread principal via la file d'événements
}



// Fonction pour appeler la méthode yield() et gérer les erreurs
static void yield() {
    rc = client->yield(100);  // Gère la communication MQTT

    if (rc != 0) {
        printf("Yield error: %d\n", rc);
        system_reset();  // Redémarre le système en cas d'erreur
    }
}

// Fonction pour publier les données sur les topics MQTT
static int8_t publish(const char* topic, float value) {
    char data[16];
    snprintf(data, sizeof(data), "%f", value);

    MQTT::Message message;
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)data;
    message.payloadlen = strlen(data);

    rc = client->publish(topic, message);
    if (rc != 0) {
        printf("Failed to publish: %d\n", rc);
        return rc;
    }
    return 0;
}

// Fonction pour publier périodiquement les données sur MQTT
void mqtt_thread() {
    while (true) {
        client->yield(100);  // Permet de gérer les messages entrants

        // Verrouille les données pour éviter les accès simultanés
        data_mutex.lock();
        float t = temperature;
        float h = humidity;
        float p = pressure;
        data_mutex.unlock();

        // Publie les données sur les topics MQTT
        publish(FEED_TEMP, t);
        publish(FEED_HUM, h);
        publish(FEED_PRESS, p);

        ThisThread::sleep_for(6s);  // Attente de 6 secondes avant de publier à nouveau
    }
}

// Fonction principale
int main() {
    led = 0;  // Éteint la LED au démarrage
    sensor.initialize();  // Initialise le capteur BME280
    sensor.set_sampling();  // Lance la lecture des données du capteur

    queue_thread.start(callback(&queue, &EventQueue::dispatch_forever));  // Lance la gestion de la file d'événements

    printf("Connecting to border router...\n");

    // Connexion au réseau
    network = NetworkInterface::get_default_instance();
    if (!network) {
        printf("Error! No network interface found.\n");
        return 0;
    }

    // Ajout d'un serveur DNS spécifique (IPv6)
    nsapi_addr_t new_dns = { NSAPI_IPv6, { 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 } };
    nsapi_dns_add_server(new_dns, "LOWPAN");

    // Connexion au réseau
    rc = network->connect();
    if (rc != 0) {
        printf("Error! net->connect() returned: %d\n", rc);
        return rc;
    }

    // Affichage de l'adresse IP obtenue
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    // Connexion au serveur MQTT
    TCPSocket socket;
    SocketAddress address;
    network->gethostbyname(hostname, &address);
    address.set_port(port);

    client = new MQTTClient(&socket);
    socket.open(network);
    rc = socket.connect(address);
    if (rc != 0) {
        printf("Connection to MQTT broker Failed\n");
        return rc;
    }

    // Connexion MQTT avec les informations d'identification
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 4;
    data.keepAliveInterval = 25;
    data.username.cstring = "Paul2sq";
    data.password.cstring = "aio_BnEn55V7Qc9WDES3QbUiqiBtxgE3";

    if (client->connect(data) != 0) {
        printf("Connection to MQTT Broker Failed\n");
        return 0;
    }

    printf("Connected to MQTT broker\n");

    // Enregistrement du callback pour le bouton
    button.rise(&on_button_rise);

    // Démarrage des threads
    sensorThread.start(sensor_thread);
    mqttThread.start(mqtt_thread);

    // Boucle infinie pour laisser le programme tourner
    while (1) {
        ThisThread::sleep_for(10ms);
    }
}
