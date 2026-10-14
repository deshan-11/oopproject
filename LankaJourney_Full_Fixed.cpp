

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cctype>

using namespace std;

namespace UI {
    void clearScreen() {
#ifdef _WIN32
        cout << "\033[2J\033[H";
#else
        cout << "\033[2J\033[H";
#endif
        cout.flush();
    }

    void pause() {
        cout << "\nPress Enter to continue...";
        string ignored;
        getline(cin, ignored);
    }

    string readLine(const string& prompt) {
        cout << prompt;
        string value;
        getline(cin, value);
        return value;
    }

    int readInt(const string& prompt, int minValue, int maxValue) {
        while (true) {
            string input = readLine(prompt);
            stringstream ss(input);
            int value;
            char extra;
            if (ss >> value && !(ss >> extra) &&
                value >= minValue && value <= maxValue) {
                return value;
            }
            cout << "Please enter a number from " << minValue
                 << " to " << maxValue << ".\n";
        }
    }

    double readDouble(const string& prompt, double minValue) {
        while (true) {
            string input = readLine(prompt);
            stringstream ss(input);
            double value;
            char extra;
            if (ss >> value && !(ss >> extra) && value >= minValue) {
                return value;
            }
            cout << "Please enter a valid amount (minimum "
                 << minValue << ").\n";
        }
    }

    void title(const string& heading) {
        clearScreen();
        cout << "============================================================\n";
        cout << "                    LANKAJOURNEY                             \n";
        cout << "              Discover the beauty of Sri Lanka              \n";
        cout << "============================================================\n";
        cout << "\n  " << heading << "\n";
        cout << "------------------------------------------------------------\n";
    }

    string askPassword(const string& prompt = "Password") {
        return readLine(prompt + " (input is visible): ");
    }
}

class User {
private:
    string name;
    string email;
    string password;
    string role;

public:
    User() : role("Customer") {}

    User(const string& n, const string& e, const string& p,
         const string& r = "Customer")
        : name(n), email(e), password(p), role(r) {}

    string getName() const { return name; }
    string getEmail() const { return email; }
    string getRole() const { return role; }

    bool checkPassword(const string& candidate) const {
        return password == candidate;
    }

    void changePassword(const string& newPassword) {
        password = newPassword;
    }

    string serialize() const {
        return name + "|" + email + "|" + password + "|" + role;
    }

    static bool deserialize(const string& line, User& user) {
        stringstream ss(line);
        string n, e, p, r;
        if (!getline(ss, n, '|')) return false;
        if (!getline(ss, e, '|')) return false;
        if (!getline(ss, p, '|')) return false;
        if (!getline(ss, r, '|')) return false;
        user = User(n, e, p, r);
        return true;
    }
};

class Booking {
private:
    int bookingId;
    string customerEmail;
    string destination;
    string travelDate;
    int guests;
    double cost;
    string status;

public:
    Booking() : bookingId(0), guests(1), cost(0.0), status("Confirmed") {}

    Booking(int id, const string& email, const string& place,
            const string& date, int people, double amount,
            const string& state = "Confirmed")
        : bookingId(id), customerEmail(email), destination(place),
          travelDate(date), guests(people), cost(amount), status(state) {}

    int getId() const { return bookingId; }
    string getCustomerEmail() const { return customerEmail; }
    double getCost() const { return cost; }
    string getStatus() const { return status; }

    void cancel() { status = "Cancelled"; }

    string serialize() const {
        ostringstream out;
        out << bookingId << "|" << customerEmail << "|" << destination
            << "|" << travelDate << "|" << guests << "|"
            << fixed << setprecision(2) << cost << "|" << status;
        return out.str();
    }

    static bool deserialize(const string& line, Booking& booking) {
        stringstream ss(line);
        string idText, email, place, date, guestsText, costText, statusText;
        if (!getline(ss, idText, '|') ||
            !getline(ss, email, '|') ||
            !getline(ss, place, '|') ||
            !getline(ss, date, '|') ||
            !getline(ss, guestsText, '|') ||
            !getline(ss, costText, '|') ||
            !getline(ss, statusText, '|')) return false;

        try {
            booking = Booking(stoi(idText), email, place, date,
                              stoi(guestsText), stod(costText), statusText);
            return true;
        } catch (...) {
            return false;
        }
    }

    void display() const {
        cout << left << setw(8) << bookingId
             << setw(24) << destination.substr(0, 22)
             << setw(14) << travelDate.substr(0, 12)
             << setw(8) << guests
             << "LKR " << setw(10) << fixed << setprecision(2) << cost
             << status << "\n";
    }
};

class Database {
private:
    vector<User> users;
    vector<Booking> bookings;
    const string usersFile = "lankajourney_users.txt";
    const string bookingsFile = "lankajourney_bookings.txt";

public:
    Database() {
        loadUsers();
        loadBookings();
        if (users.empty()) {
            users.push_back(User("Administrator", "admin@lankajourney.lk",
                                 "admin123", "Admin"));
            saveUsers();
        }
    }

    void loadUsers() {
        users.clear();
        ifstream file(usersFile);
        string line;
        while (getline(file, line)) {
            User user;
            if (User::deserialize(line, user)) users.push_back(user);
        }
    }

    void saveUsers() const {
        ofstream file(usersFile);
        for (const User& user : users) file << user.serialize() << "\n";
    }

    void loadBookings() {
        bookings.clear();
        ifstream file(bookingsFile);
        string line;
        while (getline(file, line)) {
            Booking booking;
            if (Booking::deserialize(line, booking)) bookings.push_back(booking);
        }
    }

    void saveBookings() const {
        ofstream file(bookingsFile);
        for (const Booking& booking : bookings) file << booking.serialize() << "\n";
    }

    bool emailExists(const string& email) const {
        for (const User& user : users) {
            if (user.getEmail() == email) return true;
        }
        return false;
    }

    bool addUser(const User& user) {
        if (emailExists(user.getEmail())) return false;
        users.push_back(user);
        saveUsers();
        return true;
    }

    User* authenticate(const string& email, const string& password) {
        for (User& user : users) {
            if (user.getEmail() == email && user.checkPassword(password)) {
                return &user;
            }
        }
        return nullptr;
    }

    bool updatePassword(const string& email, const string& newPassword) {
        for (User& user : users) {
            if (user.getEmail() == email) {
                user.changePassword(newPassword);
                saveUsers();
                return true;
            }
        }
        return false;
    }

    int nextBookingId() const {
        int largest = 1000;
        for (const Booking& booking : bookings)
            largest = max(largest, booking.getId());
        return largest + 1;
    }

    void addBooking(const Booking& booking) {
        bookings.push_back(booking);
        saveBookings();
    }

    vector<Booking> bookingsFor(const string& email) const {
        vector<Booking> result;
        for (const Booking& booking : bookings)
            if (booking.getCustomerEmail() == email) result.push_back(booking);
        return result;
    }

    vector<Booking>& allBookings() { return bookings; }
    const vector<User>& allUsers() const { return users; }
    void persistBookings() const { saveBookings(); }
};

class Bookable {
public:
    virtual string getName() const = 0;
    virtual double getPrice() const = 0;
    virtual string getType() const = 0;
    virtual ~Bookable() {}
};

class Destination : public Bookable {
private:
    string name;
    string description;
    double price;

public:
    Destination(const string& n, const string& d, double p)
        : name(n), description(d), price(p) {}
    string getName() const override { return name; }
    double getPrice() const override { return price; }
    string getType() const override { return "Destination"; }
    string getDescription() const { return description; }
};

class Catalog {
private:
    vector<Destination> destinations;

public:
    Catalog() {
        destinations.push_back(Destination("Ella", "Nine Arches Bridge and scenic hills", 8500));
        destinations.push_back(Destination("Sigiriya", "Ancient rock fortress and heritage", 12000));
        destinations.push_back(Destination("Kandy", "Temple of the Tooth and lake", 6500));
        destinations.push_back(Destination("Galle", "Historic fort and southern coast", 9000));
        destinations.push_back(Destination("Nuwara Eliya", "Tea gardens and cool climate", 10000));
        destinations.push_back(Destination("Mirissa", "Beach views and whale watching", 11000));
        destinations.push_back(Destination("Yala", "Wildlife safari experience", 15000));
        destinations.push_back(Destination("Anuradhapura", "Ancient city and sacred sites", 7500));
    }

    const vector<Destination>& getDestinations() const { return destinations; }

    const Destination* findDestination(int choice) const {
        if (choice < 1 || choice > static_cast<int>(destinations.size()))
            return nullptr;
        return &destinations[choice - 1];
    }

    void showDestinations() const {
        cout << left << setw(4) << "No." << setw(20) << "Destination"
             << setw(42) << "Highlights" << "Price/person\n";
        cout << string(83, '-') << "\n";
        for (size_t i = 0; i < destinations.size(); ++i) {
            cout << left << setw(4) << i + 1
                 << setw(20) << destinations[i].getName()
                 << setw(42) << destinations[i].getDescription()
                 << "LKR " << fixed << setprecision(2)
                 << destinations[i].getPrice() << "\n";
        }
    }
};

class App {
private:
    Database database;
    Catalog catalog;
    User* currentUser;

    static bool validEmail(const string& email) {
        size_t at = email.find('@');
        size_t dot = email.find('.', at == string::npos ? 0 : at);
        return at != string::npos && dot != string::npos &&
               at > 0 && dot > at + 1 && dot + 1 < email.size();
    }

    static bool validPassword(const string& password) {
        return password.size() >= 4;
    }

    void showHeader() const {
        cout << "\nLogged in as: " << currentUser->getName()
             << " (" << currentUser->getRole() << ")\n";
    }

    void signUp() {
        UI::title("CREATE AN ACCOUNT");
        string name = UI::readLine("Full name: ");
        string email = UI::readLine("Email address: ");
        if (name.empty()) {
            cout << "Name cannot be empty.\n";
            UI::pause();
            return;
        }
        if (!validEmail(email)) {
            cout << "Please enter a valid email address.\n";
            UI::pause();
            return;
        }
        if (database.emailExists(email)) {
            cout << "This email is already registered.\n";
            UI::pause();
            return;
        }

        string password = UI::askPassword("Create password (minimum 4 characters)");
        if (!validPassword(password)) {
            cout << "Password must contain at least 4 characters.\n";
            UI::pause();
            return;
        }
        string confirm = UI::askPassword("Confirm password");
        if (password != confirm) {
            cout << "Passwords do not match.\n";
            UI::pause();
            return;
        }

        if (database.addUser(User(name, email, password))) {
            cout << "\nAccount created successfully. You can now sign in.\n";
        } else {
            cout << "\nCould not create account. Please try again.\n";
        }
        UI::pause();
    }

    void signIn() {
        UI::title("SIGN IN");
        string email = UI::readLine("Email address: ");

        for (int attempt = 1; attempt <= 3; ++attempt) {
            string password = UI::askPassword("Password");
            currentUser = database.authenticate(email, password);
            if (currentUser != nullptr) {
                cout << "\nWelcome back, " << currentUser->getName() << "!\n";
                UI::pause();
                return;
            }
            cout << "Incorrect email or password. Attempts remaining: "
                 << (3 - attempt) << "\n";
            if (attempt < 3) {
                string retry = UI::readLine("Try again? (Y/N): ");
                if (!retry.empty() &&
                    (tolower(static_cast<unsigned char>(retry[0])) != 'y')) break;
            }
        }
        currentUser = nullptr;
        cout << "\nSign in unsuccessful.\n";
        UI::pause();
    }

    void browseDestinations() {
        UI::title("EXPLORE SRI LANKA");
        catalog.showDestinations();
        UI::pause();
    }

    void makeBooking() {
        UI::title("PLAN YOUR JOURNEY");
        catalog.showDestinations();

        int choice = UI::readInt("\nChoose a destination number: ",
                                 1, static_cast<int>(catalog.getDestinations().size()));
        const Destination* destination = catalog.findDestination(choice);
        if (destination == nullptr) {
            cout << "Invalid destination.\n";
            UI::pause();
            return;
        }

        string date = UI::readLine("Travel date (YYYY-MM-DD): ");
        if (date.empty()) {
            cout << "Travel date cannot be empty.\n";
            UI::pause();
            return;
        }
        int guests = UI::readInt("Number of travellers (1-20): ", 1, 20);
        double total = destination->getPrice() * guests;

        cout << "\n---------------- BOOKING SUMMARY ----------------\n";
        cout << "Destination : " << destination->getName() << "\n";
        cout << "Travel date : " << date << "\n";
        cout << "Travellers  : " << guests << "\n";
        cout << "Price/person: LKR " << fixed << setprecision(2)
             << destination->getPrice() << "\n";
        cout << "Total       : LKR " << total << "\n";

        string confirm = UI::readLine("Confirm booking? (Y/N): ");
        if (!confirm.empty() &&
            tolower(static_cast<unsigned char>(confirm[0])) == 'y') {
            Booking booking(database.nextBookingId(), currentUser->getEmail(),
                            destination->getName(), date, guests, total);
            database.addBooking(booking);
            cout << "\nBooking confirmed! Your booking ID is "
                 << booking.getId() << ".\n";
        } else {
            cout << "\nBooking cancelled. No booking was saved.\n";
        }
        UI::pause();
    }

    void myBookings() {
        UI::title("MY BOOKINGS");
        vector<Booking> bookings = database.bookingsFor(currentUser->getEmail());
        if (bookings.empty()) {
            cout << "You do not have any bookings yet.\n";
        } else {
            cout << left << setw(8) << "ID" << setw(24) << "Destination"
                 << setw(14) << "Travel date" << setw(8) << "Guests"
                 << "Total cost       Status\n";
            cout << string(80, '-') << "\n";
            for (const Booking& booking : bookings) booking.display();
        }
        UI::pause();
    }

    void cancelBooking() {
        UI::title("CANCEL A BOOKING");
        vector<Booking> bookings = database.bookingsFor(currentUser->getEmail());
        bool found = false;
        for (const Booking& booking : bookings) {
            if (booking.getStatus() == "Confirmed") {
                if (!found) {
                    cout << left << setw(8) << "ID" << setw(24) << "Destination"
                         << setw(14) << "Travel date" << setw(8) << "Guests"
                         << "Total cost       Status\n";
                    cout << string(80, '-') << "\n";
                }
                booking.display();
                found = true;
            }
        }

        if (!found) {
            cout << "There are no confirmed bookings to cancel.\n";
            UI::pause();
            return;
        }

        int id = UI::readInt("Enter booking ID to cancel: ", 1, 1000000000);
        bool cancelled = false;
        for (Booking& booking : database.allBookings()) {
            if (booking.getId() == id &&
                booking.getCustomerEmail() == currentUser->getEmail() &&
                booking.getStatus() == "Confirmed") {
                booking.cancel();
                cancelled = true;
                break;
            }
        }
        database.persistBookings();
        cout << (cancelled ? "Booking cancelled successfully.\n"
                           : "Booking ID not found among your confirmed bookings.\n");
        UI::pause();
    }

    void changePassword() {
        UI::title("CHANGE PASSWORD");
        string oldPassword = UI::askPassword("Current password");
        if (!currentUser->checkPassword(oldPassword)) {
            cout << "Current password is incorrect.\n";
            UI::pause();
            return;
        }

        string newPassword = UI::askPassword("New password (minimum 4 characters)");
        if (!validPassword(newPassword)) {
            cout << "Password must contain at least 4 characters.\n";
            UI::pause();
            return;
        }
        string confirm = UI::askPassword("Confirm new password");
        if (newPassword != confirm) {
            cout << "Passwords do not match.\n";
            UI::pause();
            return;
        }

        string email = currentUser->getEmail();
        if (database.updatePassword(email, newPassword)) {
            cout << "Password changed successfully.\n";
        } else {
            cout << "Unable to update password.\n";
        }
        UI::pause();
    }

    void adminPanel() {
        while (currentUser != nullptr && currentUser->getRole() == "Admin") {
            UI::title("ADMIN PANEL");
            cout << "1. View all bookings\n";
            cout << "2. View registered users\n";
            cout << "3. Back to main menu\n";
            int choice = UI::readInt("Select an option: ", 1, 3);

            if (choice == 1) {
                UI::title("ALL BOOKINGS");
                const vector<Booking>& bookings = database.allBookings();
                if (bookings.empty()) {
                    cout << "No bookings have been created yet.\n";
                } else {
                    cout << left << setw(8) << "ID" << setw(24) << "Destination"
                         << setw(14) << "Travel date" << setw(8) << "Guests"
                         << "Total cost       Status\n";
                    cout << string(80, '-') << "\n";
                    for (const Booking& booking : bookings) booking.display();
                }
                UI::pause();
            } else if (choice == 2) {
                UI::title("REGISTERED USERS");
                cout << left << setw(30) << "Name" << setw(38) << "Email"
                     << "Role\n";
                cout << string(75, '-') << "\n";
                for (const User& user : database.allUsers()) {
                    cout << left << setw(30) << user.getName()
                         << setw(38) << user.getEmail()
                         << user.getRole() << "\n";
                }
                UI::pause();
            } else {
                return;
            }
        }
    }

    void userMenu() {
        while (currentUser != nullptr) {
            UI::title("MAIN MENU");
            showHeader();
            cout << "\n";
            cout << "  1. Explore destinations\n";
            cout << "  2. Plan / book a journey\n";
            cout << "  3. View my bookings\n";
            cout << "  4. Cancel a booking\n";
            cout << "  5. Change password\n";
            if (currentUser->getRole() == "Admin")
                cout << "  6. Admin panel\n";
            cout << "  0. Log out\n\n";

            int choice = UI::readInt("Choose an option: ", 0, 6);
            switch (choice) {
                case 1: browseDestinations(); break;
                case 2: makeBooking(); break;
                case 3: myBookings(); break;
                case 4: cancelBooking(); break;
                case 5: changePassword(); break;
                case 6:
                    if (currentUser->getRole() == "Admin") adminPanel();
                    else cout << "Invalid option.\n";
                    break;
                case 0:
                    currentUser = nullptr;
                    cout << "\nYou have been logged out successfully.\n";
                    UI::pause();
                    return;
                default:
                    cout << "Invalid option.\n";
                    UI::pause();
            }
        }
    }

public:
    App() : currentUser(nullptr) {}

    void run() {
        while (true) {
            UI::title("WELCOME");
            cout << "  Explore destinations, plan trips, and manage bookings.\n\n";
            cout << "  1. Sign in\n";
            cout << "  2. Create an account\n";
            cout << "  3. Explore destinations as a guest\n";
            cout << "  0. Exit\n\n";

            int choice = UI::readInt("Choose an option: ", 0, 3);
            switch (choice) {
                case 1:
                    signIn();
                    if (currentUser != nullptr) userMenu();
                    break;
                case 2:
                    signUp();
                    break;
                case 3:
                    browseDestinations();
                    break;
                case 0:
                    UI::title("THANK YOU");
                    cout << "Thank you for using LankaJourney!\n";
                    cout << "Have a safe and wonderful journey.\n\n";
                    return;
            }
        }
    }
};

int main() {
    App app;
    app.run();
    return 0;
}
