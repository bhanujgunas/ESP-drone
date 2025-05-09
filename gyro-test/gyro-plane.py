import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time
import math
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

class PlaneSimulator3D:
    def __init__(self, root):
        self.root = root
        root.title("Advanced 3D Plane Simulator")
        
        # Window setup for 1920x1080 with taskbar consideration
        self.window_width = 1600
        self.window_height = 900
        screen_width = root.winfo_screenwidth()
        screen_height = root.winfo_screenheight()
        center_x = int(screen_width/2 - self.window_width/2)
        center_y = int(screen_height/2 - self.window_height/2)
        root.geometry(f'{self.window_width}x{self.window_height}+{center_x}+{center_y}')
        
        # Serial connection
        self.serial_port = None
        self.baud_rate = 115200
        self.ser = None
        self.connected = False
        self.serial_thread = None
        
        # Sensor data
        self.raw_pitch = 0.0
        self.raw_roll = 0.0
        self.raw_yaw = 0.0
        
        # Direction inversion flags
        self.invert_pitch = False
        self.invert_roll = False
        self.invert_yaw = False
        
        # Axis mapping
        self.axis_mapping = {
            'pitch': 'pitch',
            'roll': 'roll',
            'yaw': 'yaw'
        }
        
        # Create GUI
        self.create_widgets()
        
        # Initialize 3D plane model
        self.initialize_plane_model()
        
        # Auto-detect port
        self.refresh_ports()
    
    def refresh_ports(self):
        """Refresh list of available serial ports"""
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combobox['values'] = ports
        if ports and not self.port_combobox.get():
            self.port_combobox.set(ports[0])
    
    def create_widgets(self):
        # Main container using grid for better layout control
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Left panel for controls
        control_frame = ttk.Frame(main_frame, width=300)
        control_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)
        
        # Right panel for visualization
        vis_frame = ttk.Frame(main_frame)
        vis_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Connection controls
        conn_frame = ttk.LabelFrame(control_frame, text="Connection", padding=10)
        conn_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, sticky=tk.W)
        self.port_combobox = ttk.Combobox(conn_frame, width=20)
        self.port_combobox.grid(row=0, column=1, padx=5, pady=2)
        
        refresh_btn = ttk.Button(conn_frame, text="↻", width=3, command=self.refresh_ports)
        refresh_btn.grid(row=0, column=2, padx=5)
        
        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=1, column=0, columnspan=3, pady=5, sticky=tk.EW)
        
        # Calibration button
        self.calibrate_btn = ttk.Button(conn_frame, text="Calibrate", command=self.start_calibration)
        self.calibrate_btn.grid(row=2, column=0, columnspan=3, pady=5, sticky=tk.EW)
        
        # Axis mapping controls
        map_frame = ttk.LabelFrame(control_frame, text="Axis Configuration", padding=10)
        map_frame.pack(fill=tk.X, pady=5)
        
        # Pitch control
        ttk.Label(map_frame, text="Pitch:").grid(row=0, column=0, sticky=tk.W)
        self.pitch_var = tk.StringVar(value="pitch")
        pitch_combo = ttk.Combobox(map_frame, textvariable=self.pitch_var, 
                                 values=('pitch', 'roll', 'yaw'), width=8)
        pitch_combo.grid(row=0, column=1, padx=5, pady=2)
        self.pitch_invert = tk.BooleanVar(value=False)
        ttk.Checkbutton(map_frame, text="Invert", variable=self.pitch_invert).grid(row=0, column=2)
        
        # Roll control
        ttk.Label(map_frame, text="Roll:").grid(row=1, column=0, sticky=tk.W)
        self.roll_var = tk.StringVar(value="roll")
        roll_combo = ttk.Combobox(map_frame, textvariable=self.roll_var, 
                                values=('pitch', 'roll', 'yaw'), width=8)
        roll_combo.grid(row=1, column=1, padx=5, pady=2)
        self.roll_invert = tk.BooleanVar(value=False)
        ttk.Checkbutton(map_frame, text="Invert", variable=self.roll_invert).grid(row=1, column=2)
        
        # Yaw control
        ttk.Label(map_frame, text="Yaw:").grid(row=2, column=0, sticky=tk.W)
        self.yaw_var = tk.StringVar(value="yaw")
        yaw_combo = ttk.Combobox(map_frame, textvariable=self.yaw_var, 
                                values=('pitch', 'roll', 'yaw'), width=8)
        yaw_combo.grid(row=2, column=1, padx=5, pady=2)
        self.yaw_invert = tk.BooleanVar(value=False)
        ttk.Checkbutton(map_frame, text="Invert", variable=self.yaw_invert).grid(row=2, column=2)
        
        # Apply settings button
        ttk.Button(map_frame, text="Apply Settings", command=self.update_settings).grid(
            row=3, column=0, columnspan=3, pady=5, sticky=tk.EW)
        
        # Data display
        data_frame = ttk.LabelFrame(control_frame, text="Data", padding=10)
        data_frame.pack(fill=tk.X, pady=5)
        
        self.raw_data_label = ttk.Label(data_frame, text="Raw:\nPitch=0.0°\nRoll=0.0°\nYaw=0.0°")
        self.raw_data_label.pack(anchor=tk.W)
        
        self.mapped_data_label = ttk.Label(data_frame, text="Mapped:\nPitch=0.0°\nRoll=0.0°\nYaw=0.0°")
        self.mapped_data_label.pack(anchor=tk.W)
        
        # Status bar
        self.status_var = tk.StringVar(value="Disconnected")
        status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN)
        status_bar.pack(fill=tk.X, side=tk.BOTTOM)
        
        # 3D Visualization
        self.fig = Figure(figsize=(8, 6), dpi=100)
        self.ax = self.fig.add_subplot(111, projection='3d')
        self.setup_3d_axes()
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=vis_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
    
    def setup_3d_axes(self):
        """Configure 3D axes settings"""
        self.ax.set_xlim([-1.5, 1.5])
        self.ax.set_ylim([-1.5, 1.5])
        self.ax.set_zlim([-1.5, 1.5])
        self.ax.set_axis_off()
        self.ax.set_facecolor('lightblue')
        
        # Add simple horizon reference
        xx, yy = np.meshgrid(np.linspace(-2, 2, 10), np.linspace(-2, 2, 10))
        zz = np.zeros_like(xx)
        self.ax.plot_surface(xx, yy, zz, color='gray', alpha=0.2)
    
    def initialize_plane_model(self):
        """Create detailed 3D plane model"""
        # Fuselage (main body)
        self.fuselage_verts = np.array([
            [-1.0, -0.1, -0.1], [1.0, -0.1, -0.1], [1.0, 0.1, -0.1], [-1.0, 0.1, -0.1],
            [-1.0, -0.1, 0.1], [1.0, -0.1, 0.1], [1.0, 0.1, 0.1], [-1.0, 0.1, 0.1]
        ])
        
        # Main wings
        self.wing_verts = np.array([
            [-0.8, -1.2, 0.0], [0.8, -1.2, 0.0], [0.8, 1.2, 0.0], [-0.8, 1.2, 0.0]
        ])
        
        # Tail wing
        self.tail_verts = np.array([
            [-0.3, -0.4, 0.0], [0.3, -0.4, 0.0], [0.3, 0.4, 0.0], [-0.3, 0.4, 0.0]
        ])
        self.tail_pos = np.array([0.7, 0.0, 0.0])
        
        # Vertical stabilizer
        self.stabilizer_verts = np.array([
            [-0.1, 0.0, -0.1], [0.1, 0.0, -0.1], [0.1, 0.0, 0.5], [-0.1, 0.0, 0.5]
        ])
        self.stabilizer_pos = np.array([0.7, 0.0, 0.0])
        
        # Nose cone
        self.nose_verts = np.array([
            [1.0, 0.0, 0.0], [1.3, 0.0, 0.0]
        ])
        
        # Colors
        self.fuselage_color = '#2c3e50'  # Dark blue-gray
        self.wing_color = '#3498db'      # Light blue
        self.tail_color = '#e74c3c'      # Red
        self.stabilizer_color = '#f39c12' # Orange
        self.nose_color = '#ecf0f1'      # Light gray
    
    def draw_plane(self, pitch, roll, yaw):
        """Draw 3D plane with given orientation"""
        self.ax.clear()
        self.setup_3d_axes()
        
        # Apply direction inversion
        if self.invert_pitch:
            pitch = -pitch
        if self.invert_roll:
            roll = -roll
        if self.invert_yaw:
            yaw = -yaw
        
        # Convert angles to radians
        pitch_rad = math.radians(pitch)
        roll_rad = math.radians(roll)
        yaw_rad = math.radians(yaw)
        
        # Rotation matrices
        def rotate_x(verts, angle):
            rot = np.array([
                [1, 0, 0],
                [0, math.cos(angle), -math.sin(angle)],
                [0, math.sin(angle), math.cos(angle)]
            ])
            return np.dot(verts, rot)
        
        def rotate_y(verts, angle):
            rot = np.array([
                [math.cos(angle), 0, math.sin(angle)],
                [0, 1, 0],
                [-math.sin(angle), 0, math.cos(angle)]
            ])
            return np.dot(verts, rot)
        
        def rotate_z(verts, angle):
            rot = np.array([
                [math.cos(angle), -math.sin(angle), 0],
                [math.sin(angle), math.cos(angle), 0],
                [0, 0, 1]
            ])
            return np.dot(verts, rot)
        
        # Apply rotations in yaw-pitch-roll order
        def rotate_all(verts):
            rotated = rotate_z(verts, yaw_rad)
            rotated = rotate_y(rotated, pitch_rad)
            rotated = rotate_x(rotated, roll_rad)
            return rotated
        
        # Rotate all components
        fuselage = rotate_all(self.fuselage_verts)
        wings = rotate_all(self.wing_verts)
        tail = rotate_all(self.tail_verts + self.tail_pos)
        stabilizer = rotate_all(self.stabilizer_verts + self.stabilizer_pos)
        nose = rotate_all(self.nose_verts)
        
        # Create faces for fuselage (cube)
        fuselage_faces = [
            [fuselage[0], fuselage[1], fuselage[2], fuselage[3]],  # bottom
            [fuselage[4], fuselage[5], fuselage[6], fuselage[7]],  # top
            [fuselage[0], fuselage[1], fuselage[5], fuselage[4]],  # front
            [fuselage[2], fuselage[3], fuselage[7], fuselage[6]],  # back
            [fuselage[1], fuselage[2], fuselage[6], fuselage[5]],  # right
            [fuselage[0], fuselage[3], fuselage[7], fuselage[4]]   # left
        ]
        
        # Create faces for other components
        wing_face = [wings[0], wings[1], wings[2], wings[3]]
        tail_face = [tail[0], tail[1], tail[2], tail[3]]
        stabilizer_face = [stabilizer[0], stabilizer[1], stabilizer[2], stabilizer[3]]
        
        # Draw components
        fuselage_collection = Poly3DCollection(fuselage_faces, alpha=0.9)
        fuselage_collection.set_facecolor(self.fuselage_color)
        self.ax.add_collection3d(fuselage_collection)
        
        wing_collection = Poly3DCollection([wing_face], alpha=0.8)
        wing_collection.set_facecolor(self.wing_color)
        self.ax.add_collection3d(wing_collection)
        
        tail_collection = Poly3DCollection([tail_face], alpha=0.8)
        tail_collection.set_facecolor(self.tail_color)
        self.ax.add_collection3d(tail_collection)
        
        stabilizer_collection = Poly3DCollection([stabilizer_face], alpha=0.8)
        stabilizer_collection.set_facecolor(self.stabilizer_color)
        self.ax.add_collection3d(stabilizer_collection)
        
        # Draw nose
        self.ax.plot(nose[:,0], nose[:,1], nose[:,2], color=self.nose_color, linewidth=4)
        
        self.canvas.draw()
    
    def update_settings(self):
        """Update all settings from UI controls"""
        self.axis_mapping = {
            'pitch': self.pitch_var.get(),
            'roll': self.roll_var.get(),
            'yaw': self.yaw_var.get()
        }
        self.invert_pitch = self.pitch_invert.get()
        self.invert_roll = self.roll_invert.get()
        self.invert_yaw = self.yaw_invert.get()
        
        messagebox.showinfo("Settings Updated", 
                          f"New settings applied:\n"
                          f"Pitch: {self.axis_mapping['pitch']} {'(inverted)' if self.invert_pitch else ''}\n"
                          f"Roll: {self.axis_mapping['roll']} {'(inverted)' if self.invert_roll else ''}\n"
                          f"Yaw: {self.axis_mapping['yaw']} {'(inverted)' if self.invert_yaw else ''}")
    
    def toggle_connection(self):
        """Toggle serial connection"""
        if not self.connected:
            self.connect_serial()
        else:
            self.disconnect_serial()
    
    def connect_serial(self):
        """Establish serial connection"""
        port = self.port_combobox.get()
        if not port:
            messagebox.showerror("Error", "No port selected!")
            return
        
        try:
            self.ser = serial.Serial(
                port=port,
                baudrate=self.baud_rate,
                timeout=1,
                write_timeout=1
            )
            time.sleep(2)  # Wait for connection
            
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
            
            self.connected = True
            self.connect_btn.config(text="Disconnect")
            self.status_var.set(f"Connected to {port}")
            
            self.serial_thread = threading.Thread(
                target=self.read_serial,
                daemon=True
            )
            self.serial_thread.start()
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect:\n{str(e)}")
            if self.ser and self.ser.is_open:
                self.ser.close()
            self.status_var.set("Connection failed")
    
    def disconnect_serial(self):
        """Close serial connection"""
        self.connected = False
        if self.ser and self.ser.is_open:
            self.ser.close()
        self.connect_btn.config(text="Connect")
        self.status_var.set("Disconnected")
    
    def get_mapped_values(self):
        """Get values based on current mapping"""
        return {
            'pitch': getattr(self, f"raw_{self.axis_mapping['pitch']}"),
            'roll': getattr(self, f"raw_{self.axis_mapping['roll']}"),
            'yaw': getattr(self, f"raw_{self.axis_mapping['yaw']}")
        }
    
    def read_serial(self):
        """Thread for reading serial data"""
        buffer = ""
        while self.connected:
            try:
                if self.ser.in_waiting:
                    raw_data = self.ser.read(self.ser.in_waiting)
                    try:
                        buffer += raw_data.decode('ascii', errors='replace')
                    except UnicodeDecodeError:
                        continue
                    
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        
                        if line.startswith("PITCH:") and "ROLL:" in line and "YAW:" in line:
                            try:
                                parts = line.split(',')
                                self.raw_pitch = float(parts[0].split(':')[1])
                                self.raw_roll = float(parts[1].split(':')[1])
                                self.raw_yaw = float(parts[2].split(':')[1])
                                
                                mapped = self.get_mapped_values()
                                
                                self.root.after(0, lambda: self.update_display(
                                    self.raw_pitch, self.raw_roll, self.raw_yaw,
                                    mapped['pitch'], mapped['roll'], mapped['yaw']
                                ))
                                
                            except (ValueError, IndexError) as e:
                                print(f"Parse error in line: {line}")
                                continue
                
                time.sleep(0.01)
                
            except serial.SerialException as e:
                print(f"Serial error: {e}")
                self.root.after(0, self.disconnect_serial)
                break
            except Exception as e:
                print(f"Unexpected error: {e}")
                self.root.after(0, self.disconnect_serial)
                break
    
    def update_display(self, raw_pitch, raw_roll, raw_yaw, mapped_pitch, mapped_roll, mapped_yaw):
        """Update all display elements"""
        # Update raw data display
        self.raw_data_label.config(
            text=f"Raw:\nPitch={raw_pitch:.1f}°\nRoll={raw_roll:.1f}°\nYaw={raw_yaw:.1f}°")
        
        # Update mapped data display
        self.mapped_data_label.config(
            text=f"Mapped:\nPitch={mapped_pitch:.1f}°\nRoll={mapped_roll:.1f}°\nYaw={mapped_yaw:.1f}°")
        
        # Update 3D visualization
        self.draw_plane(mapped_pitch, mapped_roll, mapped_yaw)
    
    def start_calibration(self):
        """Send calibration command to ESP32"""
        if not self.connected:
            messagebox.showerror("Error", "Not connected to ESP32!")
            return
        
        try:
            self.ser.write(b'c')  # Send calibration command
            self.status_var.set("Calibrating... keep sensor flat and still")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to start calibration: {str(e)}")
    
    def on_closing(self):
        """Handle window closing"""
        self.disconnect_serial()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = PlaneSimulator3D(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()