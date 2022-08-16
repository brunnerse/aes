----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 28.07.2022 01:11:18
-- Design Name: 
-- Module Name: AHB_Interface - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.std_logic_1164.ALL;
use work.common.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity AHB_Interface is
    Port ( 
    
-- AHB Signals   
     s_ahb_hclk        : in std_logic;
     s_ahb_hresetn     : in std_logic;                     
     s_ahb_hsel        : in  std_logic;
       
     s_ahb_haddr       : in  std_logic_vector(31 downto 0); 
     s_ahb_hprot       : in  std_logic_vector(3 downto 0); -- Protection control is ignored
     s_ahb_htrans      : in  std_logic_vector(1 downto 0); 
     s_ahb_hsize       : in  std_logic_vector(2 downto 0); 
     s_ahb_hwrite      : in  std_logic; 
     s_ahb_hburst      : in  std_logic_vector(2 downto 0 );
     s_ahb_hwdata      : in  std_logic_vector(31 downto 0 );

     s_ahb_hready_out  : out std_logic; 
     s_ahb_hready      : in  std_logic; 
                      
     s_ahb_hrdata      : out std_logic_vector(31 downto 0 );
     s_ahb_hresp       : out std_logic;


-- to banked registers
    key : out std_logic_vector(KEY_SIZE-1 downto 0);
    IV_or_counter : out std_logic_vector(KEY_SIZE-1 downto 0);
    status : in std_logic_vector(31 downto 0);
    control : out std_logic_vector(31 downto 0);
    data_in : out std_logic_vector(31 downto 0);
    data_out : in std_logic_vector(31 downto 0)
    );
end AHB_Interface;

architecture Behavioral of AHB_Interface is

-- Define ahb ports as xiling AHB_INTERFACE
ATTRIBUTE X_INTERFACE_INFO : STRING;
ATTRIBUTE X_INTERFACE_PARAMETER : STRING;
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hresp: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HRESP";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hrdata: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HRDATA";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hready: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HREADY";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hready_out: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HREADY_OUT";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hwdata: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HWDATA";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hburst: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HBURST";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hwrite: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HWRITE";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hsize: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HSIZE";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_htrans: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HTRANS";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hprot: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HPROT";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_haddr: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE HADDR";
ATTRIBUTE X_INTERFACE_PARAMETER OF s_ahb_hsel: SIGNAL IS "XIL_INTERFACENAME AHB_INTERFACE, BD_ATTRIBUTE.TYPE INTERIOR";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hsel: SIGNAL IS "xilinx.com:interface:ahblite:2.0 AHB_INTERFACE SEL";
ATTRIBUTE X_INTERFACE_PARAMETER OF s_ahb_hresetn: SIGNAL IS "XIL_INTERFACENAME AHB_RESETN, POLARITY ACTIVE_LOW, INSERT_VIP 0";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hresetn: SIGNAL IS "xilinx.com:signal:reset:1.0 AHB_RESETN RST";
ATTRIBUTE X_INTERFACE_PARAMETER OF s_ahb_hclk: SIGNAL IS "XIL_INTERFACENAME AHB_CLK, ASSOCIATED_BUSIF AHB_INTERFACE:M_AXI, ASSOCIATED_RESET s_ahb_hresetn, FREQ_HZ 100000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN BusTesting_Clock, INSERT_VIP 0";
ATTRIBUTE X_INTERFACE_INFO OF s_ahb_hclk: SIGNAL IS "xilinx.com:signal:clock:1.0 AHB_CLK CLK";
  

constant DATA_WIDTH : integer := 32;
constant ADDR_WIDTH : integer := 32;

constant ADDR_BASE : integer := 16#0#;
constant ADDR_CR_OFFSET : integer := 16#00#;
constant ADDR_SR_OFFSET : integer := 16#04#;
constant ADDR_DINR_OFFSET : integer := 16#08#;
constant ADDR_DOUTR_OFFSET : integer := 16#0c#;
constant ADDR_KEYR0_OFFSET : integer := 16#10#;
constant ADDR_KEYR1_OFFSET : integer := 16#14#;
constant ADDR_KEYR2_OFFSET : integer := 16#18#;
constant ADDR_KEYR3_OFFSET : integer := 16#1c#;
constant ADDR_IVR0_OFFSET : integer := 16#20#;
constant ADDR_IVR1_OFFSET : integer := 16#24#;
constant ADDR_IVR2_OFFSET : integer := 16#28#;
constant ADDR_IVR3_OFFSET : integer := 16#2c#;
constant ADDR_SUSPxR_OFFSET : integer := 16#40#;

constant HTRANS_TYPE_IDLE : std_logic_vector(1 downto 0) := "00";
constant HTRANS_TYPE_BUSY : std_logic_vector(1 downto 0) := "01";
constant HTRANS_TYPE_NONSEQ : std_logic_vector(1 downto 0) := "10";
constant HTRANS_TYPE_SEQ  : std_logic_vector(1 downto 0) := "11";



  
type state_type is (IdleOrRead, Write, BusyWrite);
signal state : state_type;

type addr_range is array (0 to 41) of std_logic_vector(ADDR_WIDTH-1 downto 0); -- TODO how many elements?
signal mem : addr_range;

begin

-- Map internal registers to the memory addresses
key <= mem(ADDR_KEYR0_OFFSET/4) & mem(ADDR_KEYR1_OFFSET/4) & mem(ADDR_KEYR2_OFFSET/4) & mem(ADDR_KEYR3_OFFSET/4);
IV_or_counter <= mem(ADDR_IVR0_OFFSET/4) & mem(ADDR_IVR1_OFFSET/4) & mem(ADDR_IVR2_OFFSET/4) & mem(ADDR_IVR3_OFFSET/4);
control <= mem(ADDR_CR_OFFSET/4);
mem(ADDR_SR_OFFSET/4) <= status;
data_in <= mem(ADDR_DINR_OFFSET/4);
mem(ADDR_DOUTR_OFFSET/4) <= data_out;


process (s_ahb_hresetn, s_ahb_hclk)
    variable local_addr : unsigned(ADDR_WIDTH-1 downto 0);
    variable index : integer;
begin
if s_ahb_hresetn = '0' then
    state <= IdleOrRead;
    s_ahb_hready_out <= '1';
    -- TODO set control signals
    control <= x"00000000";
elsif rising_edge(s_ahb_hclk) then
    -- This subordinate never has to insert wait states, so hready_out is always 1
    s_ahb_hready_out <= '1';
    
    if s_ahb_hsel = '1' then
        case state is
            when IdleOrRead =>
                case s_ahb_htrans is
                    when HTRANS_TYPE_IDLE | HTRANS_TYPE_BUSY => -- ignore idle 
                        s_ahb_hresp <= '0';
                    -- bursts don't have to be considered for read accesses
                    when HTRANS_TYPE_NONSEQ | HTRANS_TYPE_SEQ => 
                        -- Calculate index in mem array from the address
                        local_addr := unsigned(s_ahb_haddr) - to_unsigned(ADDR_BASE, 32);
                        index := to_integer(local_addr(ADDR_WIDTH-1 downto 2)); -- divide by 4
                        -- Check hwrite signal 
                        if s_ahb_hwrite = '1' then
                            -- Write: read from hwdata in the next cycle (data phase)
                            state <= Write; 
                        else
                            -- Read: Set hrdata in the same cycle
                            s_ahb_hrdata <= mem(index);
                            s_ahb_hready_out <= '1';
                            s_ahb_hresp <= '0';
                        end if;
                    when others =>
                end case;
            when Write =>
                -- TODO necessary to wait for hready signal to be high?
                
                -- Write strobes are not supported
                mem(index) <= s_ahb_hwdata;
                s_ahb_hready_out <= '1';
                s_ahb_hresp <= '0';
                -- Check control data if burst is not yet finished
                case s_ahb_htrans is
                    when HTRANS_TYPE_SEQ =>
                   -- Calculate index in mem array from the address
                    local_addr := unsigned(s_ahb_haddr) - to_unsigned(ADDR_BASE, 32);
                    index := to_integer(local_addr(ADDR_WIDTH-1 downto 2)); -- divide by 4
                when HTRANS_TYPE_BUSY =>
                    -- Write burst transfer is interrupted
                    state <= BusyWrite;
                when others => -- burst finished
                    state <= IdleorRead;
                end case;
            when BusyWrite =>
                case s_ahb_htrans is
                    when HTRANS_TYPE_SEQ =>
                        -- Writing continues in next cycle
                        -- Calculate index in mem array from the address
                        local_addr := unsigned(s_ahb_haddr) - to_unsigned(ADDR_BASE, 32);
                        index := to_integer(local_addr(ADDR_WIDTH-1 downto 2)); -- divide by 4
                        state <= Write;
                    when HTRANS_TYPE_BUSY =>
                        -- stay in BusyWrite state
                    when others =>
                        -- Burst has ended
                        state <= IdleOrRead;
                end case;
            when others =>
        end case;
    else 
        -- Select signal is low
        state <= IdleOrRead;
    end if;
end if;

end process;


end Behavioral;
